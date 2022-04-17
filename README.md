# YAUBOB
Six output yet another USB-based output board (YAUBOB)

This is yet another reinvention of the wheel.  This wheel is just as round as any other wheel.  The only difference between this and other such wheels is that this one is mine.  

It's licensed in such a way as to encourage reuse in hopes that some sort of standard develops.  I dream of a world where the hoarde of USB IO boards on EBay speak one language.

I chose to base this around an Arduino because I get USB stack, processor, software libraries etc etc. Also because I had a couple Nanos lying around waiting to be used :)

# Interface
Communication is done via standard serial port.  On my Linux machine it's /dev/ttyUSB0.  Port settings: 115200 8N1.  All commands are terminated via \n.  Documentation is available within the software by using command ? or h or v.

Example output

```
ii Serial init complete.
ii Maximum X: 63(0x3f)
!! Feed me a kitten!
?
ii Command as we understand: [?]
ii --- Start of Help ---
ii      This is YAUBOB v1.0
ii      All command must be terminated by either an \n or \r.
ii       v|?|h - Help; this
ii       r - Reboot/reset
ii       d - Demo mode
ii       s X - Set outputs where X is the bitmap in decimal form. Ex. s 1 will turn on first output.  s 2 will turn on second.  s 3 will turn on first and second and so forth in that manner.
ii --- End of Help ---
r
ii Command as we understand: [r]
ii We are rebooting

ii Serial init complete.
ii Maximum X: 63(0x3f)
!! Feed me a kitten!
s 1
ii Command as we understand: [s 1]
ii Parameter as we understand it: [1 (0x1)]
ii State: 1 (0x1)
s 44
ii Command as we understand: [s 44]
ii Parameter as we understand it: [44 (0x2c)]
ii State: 44 (0x2c)
s 100million
ii Command as we understand: [s 100million]
ii Parameter as we understand it: [100 (0x64)]
!! Output state clipped to 0x3f
ii State: 63 (0x3f)
s one
ii Command as we understand: [s one]
ii Parameter as we understand it: [0 (0x0)]
ii State: 0 (0x0)
give me snakcs!
ii Command as we understand: [give me snakcs!]
!! Unrecognized command: [give me snakcs!].  Will print help.
ii --- Start of Help ---
ii      This is YAUBOB v1.0
ii      All command must be terminated by either an \n or \r.
ii       v|?|h - Help; this
ii       r - Reboot/reset
ii       d - Demo mode
ii       s X - Set outputs where X is the bitmap in decimal form. Ex. s 1 will turn on first output.  s 2 will turn on second.  s 3 will turn on first and second and so forth in that manner.
ii --- End of Help ---
```

All output from the software has either a ```ii ``` or ```!! ```  as the first three characters (the third/last character is a space).  Any output beginning with ```ii``` is informational.  Any output beginning with ```!!``` is error.

# Home
https://github.com/vic-simkus/YAUBOB

# Author
Vidas Simkus (vic.simkus@simkus.com)

https://www.simkus.com

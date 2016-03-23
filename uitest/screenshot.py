#!/usr/bin/env python

import sys

import pyscreenshot as ImageGrab

if __name__ == "__main__":
    # part of the screen
    if len(sys.argv) != 5:
        sys.exit(1)
    im=ImageGrab.grab(bbox=(int(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3]),int(sys.argv[4]))) # X1,Y1,X2,Y2
    im.show()

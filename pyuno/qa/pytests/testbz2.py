import os
import unittest

# tdf#116412: make sure importing bz2 works on all platforms
class BZ2Test(unittest.TestCase):
    def test_bz2_import(self):
        import bz2

        # use imported bz2 module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(bz2), file=devnull)

if __name__ == '__main__':
    unittest.main()

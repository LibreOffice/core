import os
import unittest

# tdf#162786: make sure importing pip works on all platforms
class SetupToolsTest(unittest.TestCase):
    def test_pip_import(self):
        import pip

        # use imported pip module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(pip), file=devnull)

if __name__ == '__main__':
    unittest.main()

import os
import unittest

# tdf#161947: make sure importing setuptools works on all platforms
class SetupToolsTest(unittest.TestCase):
    def test_setuptools_import(self):
        import setuptools

        # use imported setuptools module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(setuptools), file=devnull)

if __name__ == '__main__':
    unittest.main()

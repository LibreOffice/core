import os
import unittest

# I want to ensure that import venv works on all platforms
class VENVTest(unittest.TestCase):
    def test_venv_import(self):
        import venv

        # use imported venv module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(venv), file=devnull)

if __name__ == '__main__':
    unittest.main()

import os
import unittest


# I want to ensure that import ssl works on all platforms
class SSLTest(unittest.TestCase):
    def test_ssl_import(self):
        import ssl

        # use imported ssl module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(ssl), file=devnull)

if __name__ == '__main__':
    unittest.main()

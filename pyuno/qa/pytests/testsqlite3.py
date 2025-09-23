import os
import unittest

# I want to ensure that import sqlite3 works on all platforms
class SQLITE3Test(unittest.TestCase):
    def test_sqlite3_import(self):
        import sqlite3

        # use imported sqlite3 module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(sqlite3), file=devnull)

if __name__ == '__main__':
    unittest.main()

import os
import unittest

class ImportsTest(unittest.TestCase):
    def test_sqlite3_import(self):
        import sqlite3

        # use imported sqlite3 module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(sqlite3), file=devnull)

    # tdf#116412: make sure importing bz2 works on all platforms
    def test_bz2_import(self):
        import bz2

        # use imported bz2 module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(bz2), file=devnull)

    # tdf#161947: make sure importing setuptools works on all platforms
    def test_setuptools_import(self):
        import setuptools

        # use imported setuptools module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(setuptools), file=devnull)

    def test_ssl_import(self):
        import ssl

        # use imported ssl module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(ssl), file=devnull)

    # tdf#162786: make sure importing pip works on all platforms
    def test_pip_import(self):
        import pip

        # use imported pip module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(pip), file=devnull)

    def test_venv_import(self):
        import venv

        # use imported venv module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(venv), file=devnull)

    def test_dbm_import(self):
        import dbm

        # use imported dbm module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(dbm), file=devnull)

    def test__hashlib_import(self):
        import _hashlib

        # use imported _hashlib module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(_hashlib), file=devnull)

    def test_lzma_import(self):
        import lzma

        # use imported lzma module for pyflakes
        with open(os.devnull, "w") as devnull:
            print(str(lzma), file=devnull)

if __name__ == '__main__':
    unittest.main()

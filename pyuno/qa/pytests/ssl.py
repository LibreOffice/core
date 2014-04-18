import unittest

#I want to ensure that import ssl works on all platforms
class SSLTest(unittest.TestCase):
    def test_ssl_import(self):
        import ssl

if __name__ == '__main__':
    unittest.main()


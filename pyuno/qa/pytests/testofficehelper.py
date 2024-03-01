import unittest
from officehelper import bootstrap, BootstrapException


class OfficeHelperTest(unittest.TestCase):
    """officehelper.py must provide:
    Support of Windows, Mac OS X & GNU/Linux distributions
    Customizable connection with 'delays' **kwarg
    Reporting to console with 'report' **kwarg
    Memory cleanup from soffice service
    extra features may be:
    Python source documentation
    """
    def test_default_config(self):
        # Check default timeout and number of attempts
        # Stop LibreOffice running service
        ctx = bootstrap()  # Default settings suffice to initialize the service
        #time.sleep(10)  # gve
        if ctx:  # stop soffice as a service
            smgr = ctx.getServiceManager()
            desktop = smgr.createInstanceWithContext("com.sun.star.frame.Desktop", ctx)
            desktop.terminate()
        self.assertTrue(ctx)  # check for failure

    def test_kwargs(self):
        # Wait differently for LO to start, request context 10 times
        # Report processing in console
        ctx = bootstrap(delays=[1,]*10, report=print)
        if ctx:  # stop soffice as a service
            smgr = ctx.getServiceManager()
            desktop = smgr.createInstanceWithContext("com.sun.star.frame.Desktop", ctx)
            desktop.terminate()
        self.assertTrue(ctx)

    def test_exception(self):
        # Raise BootstrapException and stop ALL PRESENT LibreOffice running services
        with self.assertRaises(BootstrapException):
            bootstrap(delays=[0,], report=print)  # delays=[0,] must raise BootstrapException


if __name__ == "__main__":

    unittest.main()

    # ~ dir(__name__)
    # ~ help(__name__)
    # ~ help(bootstrap)
    # ~ exit()

# vim: set shiftwidth=4 softtabstop=4 expandtab

import unittest
from officehelper import bootstrap, BootstrapException, SessionManager
from com.sun.star.frame import theDesktop

import itertools


class OfficeHelperTest(unittest.TestCase):
    """officehelper.py must provide:

    - Support of Windows, Mac OS X & GNU/Linux distributions
    - Customizable connection with 'delays' **kwarg
    - Reporting to console with 'report' **kwarg
    - Memory cleanup from soffice service
    - Process cleanup using SessionManager context manager

    extra features may be:
    Python source documentation """
    def test_kwargs(self):
        # Wait differently for LO to start, requesting the context an infinite number of times with
        # a one second delay between each request. We need to have a long delay to allow for slow
        # startup times under heavy load on Jenkins.
        # Report processing in console
        ctx = bootstrap(delays=itertools.repeat(1), report=print)
        if ctx:  # stop soffice as a service
            desktop = theDesktop.get(ctx)
            desktop.terminate()
        self.assertTrue(ctx)

    def test_exception(self):
        # Raise BootstrapException and stop ALL PRESENT LibreOffice running services
        with self.assertRaises(BootstrapException):
            bootstrap(delays=[0,], report=print)  # delays=[0,] must raise BootstrapException

    def test_session_manager(self):
        # Start/Stop a LibreOffice `randomly named` pipe service
        # The infinite number of delays are added to allow for slow startup times under heavy load
        # on Jenkins.
        with SessionManager(delays=itertools.repeat(1)) as ctx:
            self.assertTrue(theDesktop.get(ctx))

if __name__ == "__main__":

    unittest.main()

    # ~ dir(__name__)
    # ~ help(__name__)
    # ~ help(bootstrap)
    # ~ exit()

# vim: set shiftwidth=4 softtabstop=4 expandtab
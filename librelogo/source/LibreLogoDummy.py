# dummy UNO component to get the path of the localizations
import uno, unohelper, os, sys
from com.sun.star.lang import XMain
class LibreLogoDummyComponent(XMain, unohelper.Base):
    def __init__( self, ctx ):
        self.ctx = ctx

    def get_path(self):
        import librelogodummy_path
        return librelogodummy_path.get_path()

g_ImplementationHelper = unohelper.ImplementationHelper()
g_ImplementationHelper.addImplementation( \
    LibreLogoDummyComponent,"org.openoffice.comp.pyuno.LibreLogoDummy",("org.openoffice.LibreLogo.LibreLogoDummy",),)

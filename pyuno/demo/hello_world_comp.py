import uno
import unohelper

from com.sun.star.task import XJobExecutor

# implement a UNO component by deriving from the standard unohelper.Base class
# and from the interface(s) you want to implement.
class HelloWorldJob( unohelper.Base, XJobExecutor ):
    def __init__( self, ctx ):
        # store the component context for later use
        self.ctx = ctx
        
    def trigger( self, args ):
        # note: args[0] == "HelloWorld", see below config settings
    
        # retrieve the desktop object
        desktop = self.ctx.ServiceManager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", self.ctx )
	    
        # get current document model
        model = desktop.getCurrentComponent()

	# access the document's text property
	text = model.Text

	# create a cursor
	cursor = text.createTextCursor()

	# insert the text into the document
	text.insertString( cursor, "Hello World", 0 )

# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper()

# 
g_ImplementationHelper.addImplementation( \
	HelloWorldJob,                        # UNO object class
	"org.openoffice.comp.pyuno.demo.HelloWorld", # implemenation name 
	("com.sun.star.task.Job",),)          # list of implemented services
	                                      # (the only service)

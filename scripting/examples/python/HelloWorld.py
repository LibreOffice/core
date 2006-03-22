# HelloWorld python script for the scripting framework

def HelloWorldPython( ):
    """Prints the string 'Hello World(in Python)' into the current document"""
#get the doc from the scripting context which is made available to all scripts
    model = XSCRIPTCONTEXT.getDocument()
#get the XText interface
    text = model.Text
#create an XTextRange at the end of the document
    tRange = text.End
#and set the string
    tRange.String = "Hello World (in Python)"
    return None

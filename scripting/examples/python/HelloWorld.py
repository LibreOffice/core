# HelloWorld python script for the scripting framework

def HelloWorldPython( ):
    """Prints the string 'Hello World(in Python)' into the current document"""
#get the doc from the scripting context which is made available to all scripts
    model = XSCRIPTCONTEXT.getDocument()
#get the XText interface
    text = model.Text
#create an XTextCursor
    cursor = text.createTextCursor()
#and insert the string
    text.insertString( cursor, "Hello World (in Python)", 0 )
    return None


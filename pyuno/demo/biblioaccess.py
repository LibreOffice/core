import uno

from com.sun.star.sdb.CommandType import COMMAND

def main():

    connectionString = "socket,host=localhost,port=2002"
    
    url = "uno:"+connectionString + ";urp;StarOffice.ComponentContext"
    
    localCtx = uno.getComponentContext()
    localSmgr = localCtx.ServiceManager
    resolver = localSmgr.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", localCtx)
    ctx = resolver.resolve( url )
    smgr = ctx.ServiceManager

    rowset =smgr.createInstanceWithContext( "com.sun.star.sdb.RowSet", ctx )
    rowset.DataSourceName = "Bibliography"
    rowset.CommandType = COMMAND
    rowset.Command = "SELECT IDENTIFIER, AUTHOR FROM biblio"

    rowset.execute();

    print "Identifier\tAuthor"

    id = rowset.findColumn( "IDENTIFIER" )
    author = rowset.findColumn( "AUTHOR" )
    while rowset.next():
        print rowset.getString( id ) + "\t" + repr( rowset.getString( author ) )


    rowset.dispose();

main()

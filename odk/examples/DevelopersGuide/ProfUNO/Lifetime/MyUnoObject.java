class MyUnoObject implements com.sun.star.uno.XInterface
{
    public MyUnoObject() {}
    void finialize() { System.out.println( "finalizer called" ); }

    public static void main( String args[] ) throws java.lang.InterruptedException
    {
        com.sun.star.uno.XInterface a = new MyUnoObject();
        a = null;

        // ask the garbage collector politely
        System.gc();
        synchronized(  Thread.currentThread() )
        {
            // wait a second
            Thread.currentThread().wait( 1000 );
        }
        System.out.println( "leaving" );

        // It is java VM dependend, whether the finalizer was called or not
    }
}

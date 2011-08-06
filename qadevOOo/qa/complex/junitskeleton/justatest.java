/**
 * @author: ll93751
 * @copyright: Sun Microsystems Inc. 2010
 */

package complex.junitskeleton;

public class justatest /* extends *//* implements */ {
    //public static void main( String[] argv ) {
    //
    //    }
    public void justatest()
        {
            System.out.println("justatest CTor.");
        }

    public void testfkt()
        {
            System.out.println("Test called.");
        }

      /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    public static void shortWait()
    {
        try
        {
            Thread.sleep(500) ;
        }
        catch (InterruptedException e)
        {
            System.out.println("While waiting :" + e) ;
        }
    }

}

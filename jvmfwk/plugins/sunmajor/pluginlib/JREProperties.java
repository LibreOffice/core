import java.io.*;
import java.util.*;
import java.awt.*;

public class JREProperties
{
    static public void main(String[] args)
    {
        try
        {
            try{
                //This line is needed to get the accessibility properties
                Toolkit tk = java.awt.Toolkit.getDefaultToolkit();
            }
            catch(Throwable e)
            {
                System.err.println(e);
            }

            Properties p = System.getProperties();
            Enumeration e = p.propertyNames();
            for (; e.hasMoreElements() ;) {
                String sProp = (String) e.nextElement();
                String sCompleteProp = sProp + "=" + p.getProperty(sProp);

                System.out.println(sCompleteProp);
            }
        }
        catch(Exception e)
        {
            System.err.println(e);
        }

        System.exit(0);
    }
}

import java.io.*;
import java.util.*;
import java.awt.*;

/** This class prints out the system properties.

    We cannot print the strings directly because of encoding issues. Since
    about 1.3.1 one can start java with the option -Dfile.encoding=UTF-8, but
    unfortunately this works only with later update - versions (for example,
    1.3.1_07). Therefore we use this scheme. The property string has this form:
    name=value

    Every character is cast to an integer which value is printed, followed by a
    space. If all characters of the string are printed, then a new line is printed.
*/
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
                char[] arChars = new char[sCompleteProp.length()];
                sCompleteProp.getChars(0, sCompleteProp.length(), arChars, 0);
                for (int c = 0; c < arChars.length; c++) {
                    System.out.print(String.valueOf((int) arChars[c]));
                    System.out.print(" ");
                }
                System.out.print("\n");
            }
        }
        catch(Exception e)
        {
            System.err.println(e);
        }

        System.exit(0);
    }
}

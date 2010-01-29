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
            boolean bNoAccess = false;
            if(args.length > 0)
            {
                if (args[0].equals("noaccessibility"))
                    bNoAccess = true;
            }

            //Find out on what operation system we are running. On Windows 98
            //we must not call getDefaultToolkit, because the office may freeze
            //#i44608.
            boolean bW98 = false;
            String os = System.getProperty("os.name");

            if (os != null)
            {
                os = os.trim();
                if (os.equalsIgnoreCase("Windows 98") ||
                    os.indexOf("Windows 98") != -1)
                    bW98 = true;
            }

            //We need to be able to switch this part off because
            //it causes an exception if the DISPLAY variable has
            //a false value. Setting the noaccessibility argument
            //can be done by providing a sunjavaplugin.ini with
            //the bootstrap parameter JFW_PLUGIN_NO_NOT_CHECK_ACCESSIBILITY
            //set to "1"
            if (bNoAccess == false && ! bW98)
            {
                try{
                    //This line is needed to get the accessibility properties
                    Toolkit tk = java.awt.Toolkit.getDefaultToolkit();
                }
                catch(Throwable e)
                {
                    System.err.println(e);
                }
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


/**
 * Title:        Import of configuration schemas<p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */
package org.openoffice.configuration;

import java.util.*;
import java.io.*;

class Trim
{
    public static void main (String argv [])
    {
        try
        {
            BufferedReader reader = new BufferedReader(new FileReader(argv[0]));
            FileWriter out = new FileWriter(argv[0] + ".trim" ,false);
            String line = "";
            boolean hadLine = false;
            while ((line = reader.readLine()) != null)
            {
                if (line.trim().length() > 0)
                {
                    hadLine = true;
                    out.write(line);
                    out.write("\n");
                }
                else if (hadLine)
                {
                    out.write("\n");
                    hadLine = false;
                }
            }
            out.flush();
            out.close();
            reader.close();

            new File(argv[0]).delete();
            new File(argv[0] + ".trim").renameTo(new File(argv[0]));
        }
        catch (Exception e)
        {}
    }
}
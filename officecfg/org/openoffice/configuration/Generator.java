
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
import com.jclark.xsl.sax.Driver;

public class Generator {

    public Generator() throws Exception
    {
        // set the driver for xt
        System.setProperty("com.jclark.xsl.sax.parser", "com.sun.xml.parser.Parser");
    }

    /**
     *  creating a abs path of a source path
     */
    static public String getAbsolutePath(String orgPath) throws IOException
    {
        String absolutePath = new String();
        if (orgPath.length() > 0)
        {
            StringTokenizer tokenizer = new StringTokenizer(orgPath, File.pathSeparator);
            absolutePath = new File(tokenizer.nextToken()).getAbsoluteFile().toURL().toString();
            while (tokenizer.hasMoreTokens())
            {
                absolutePath += File.pathSeparator;
                absolutePath += tokenizer.nextToken();
            }
        }
        return absolutePath;
    }

    public void generate(String argv []) throws Exception
    {
        try
        {
            // make sure that all directories exist
            argv[2] = new File(argv[2]).getAbsoluteFile().toString();
            File path = new File(argv[2]).getParentFile().getAbsoluteFile();
            path.mkdirs();

            Driver.main(argv);

            String[] args = new String[1];
            args[0] = argv[2];
            Trim.main(args);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            throw e;
        }
    }

    public static void main (String argv [])
    {
        if (argv.length < 4) {
            System.err.println ("Usage: cmd <filename> <xsl-file> <out-file> <include-path> [transformation parameters]");
            System.err.println ("<filename>: Configuration description file");
            System.err.println ("<xsl-file>: transformation file");
            System.err.println ("<out-file>: output file");
            System.err.println ("<include-path>: Path where to find imported configuration files");
            System.exit (1);
        }

        try
        {
            Generator generator = new Generator();

            String[] args = new String[argv.length + 1];
            for (int i = 0; i < argv.length; i++)
                args[i] = argv[i];

            // handle the path parameter for the source
            args[3] = "path=" + getAbsolutePath(argv[3]);
            args[argv.length] = "pathSeparator=" + File.pathSeparator;

            // create the instance file
            generator.generate((String[])args.clone());
        }
        catch (Exception pce) {
            // Parser with specified options can't be built
            pce.printStackTrace();
            System.exit (1);
        }
    }
}

/**
 * Title:        workaround for xslt bug with default namespaces
 * Description:  removes the first default namespace from a given xml file<p>
 * Copyright:    null<p>
 * Company:      null<p>
 * @author       Svante Schubert
 * @version      1.0
 */
package org.openoffice.helper;

import java.io.*;


public class DefaultNamespaceRemover
{
    private static final boolean debug = false;

    public static void main (String argv [])
    {

        if (argv.length != 2){
            System.err.println("Usage: ");
            System.err.println("DefaultNamespaceRemover <inputfile> <outputfile>");
            System.exit(1);
        }

        //******************************
        //parse and write the html file
        //******************************
        try{

            BufferedReader  in                  = new BufferedReader(new InputStreamReader (new FileInputStream (argv[0]), "UTF8"));
            BufferedWriter  out                 = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(argv[1]), "UTF8"));
            String          sLine               = null;
            StringBuffer    sBuffer             = null;
            boolean         isNamespaceRemoved  = false;
            int             nodeIndex1          = -1;
            int             nodeIndex2          = -1;
            int             delimiterIndex1     = -1;
            int             delimiterIndex2     = -1;
            String          lineEnd             =  System.getProperty("line.separator");

            while((sLine = in.readLine()) != null)
            {
                // as long the namespace has to be removed
                if(isNamespaceRemoved){
                    out.write(sLine + lineEnd);
                    if(debug) System.out.println(sLine + lineEnd);
                }else{
                    //see if a defaultnamespace exists in the input line
                    if((nodeIndex1 = sLine.indexOf("xmlns=")) != -1 || (nodeIndex2 = sLine.indexOf("xmlns =")) != -1)
                    {
                        // take the valid starting point of the default namespace node
                        if(nodeIndex1 == -1)
                            nodeIndex1 = nodeIndex2;

                        // for possible delimiter " see if it exist after default namespace node, then get the next
                        if((delimiterIndex1 = sLine.indexOf('\"', nodeIndex1)) != -1)
                            delimiterIndex1 = sLine.indexOf('\"', delimiterIndex1 + 1);

                        // for possible delimiter ' see if it exist after default namespace node, then get the next
                        if((delimiterIndex2 = sLine.indexOf('\'', nodeIndex2)) != -1)
                            delimiterIndex2 = sLine.indexOf('\'', delimiterIndex2 + 1);

                        // if the first delimiter ('"') does not exist in string, take the other for granted
                        if(delimiterIndex1 == -1)
                            delimiterIndex1 = delimiterIndex2;
                        // otherwise both delimiters exist, so get the first
                        else if(delimiterIndex2 != -1)
                            if(delimiterIndex2 < delimiterIndex1)
                                delimiterIndex1 = delimiterIndex2;

                        sBuffer = new StringBuffer(sLine);
                        // +2 for the delimiter itself and the following space
                        sBuffer.delete(nodeIndex1, delimiterIndex1 + 2);
                        out.write(new String(sBuffer) + lineEnd);
                        if(debug) System.out.println(new String(sBuffer) + lineEnd);
                        isNamespaceRemoved = true;
                    }else{
                        out.write(sLine + lineEnd);
                        if(debug) System.out.println(sLine + lineEnd);
                    }
                }
            }
            in.close();
            out.flush();
            if(out != null)
                out.close();
        }
        catch(Exception e)
        {
            System.out.println("Error by parsing the html file: "+e.getMessage());
            e.printStackTrace();
        }
    }
}

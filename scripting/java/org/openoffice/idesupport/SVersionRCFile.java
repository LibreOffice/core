package org.openoffice.idesupport;

import java.io.*;
import java.util.*;

public class SVersionRCFile {

    public static final String DEFAULT_NAME =
        System.getProperty("os.name").startsWith("Windows") == true ?
            System.getProperty("user.home") + File.separator +
            "Application Data" + File.separator + "sversion.ini" :
            System.getProperty("user.home") + File.separator +
            ".sversionrc";

    private static final String VERSIONS_LINE = "[Versions]";
    private static final String UNOILJAR = "skip_registration" + File.separator + "unoil.jar";
    private static final String UNOPACKAGEDIR = File.separator + "user" +
                                                File.separator + "uno_packages" +
                                                File.separator + "cache" +
                                                File.separator + "uno_packages";
    /* Make sure this is in LowerCase !!!!! */
    private static final String SCRIPTF = "scriptf";
    private static final String FILE_URL_PREFIX = "file://";
    private File file = null;

    public SVersionRCFile() {
        this(DEFAULT_NAME);
    }

    public SVersionRCFile(String name) {
        file = new File(name);
    }

    public Hashtable getVersions() throws IOException {
        BufferedReader br;

        try {
            br = new BufferedReader(new FileReader(file));
        }
        catch (FileNotFoundException fnfe) {
            throw new IOException(fnfe.getMessage());
        }

        Hashtable versions = load(br);
        br.close();
        return versions;
    }

    private Hashtable load(BufferedReader br) throws IOException {
        Hashtable versions = new Hashtable();
        String s;

        while ((s = br.readLine()) != null &&
              (s.equals(VERSIONS_LINE)) != true);

        while ((s = br.readLine()) != null &&
              (s.equals("")) != true) {
            StringTokenizer tokens = new StringTokenizer(s, "=");
            int count = tokens.countTokens();

            if (count != 2)
                continue;

            String name = tokens.nextToken();
            String path = tokens.nextToken();
            if (path.startsWith(FILE_URL_PREFIX) == true)
                path = path.substring(FILE_URL_PREFIX.length());

            if (isValidPath(path) == true)
                versions.put(name, path);
        }
        return versions;
    }

    private boolean isValidPath(String path) {
        // System.out.println("Testing: " + path);
        File file = new File(path + File.separator + "program");
        if (file.exists())
            return true;
        return false;
    }

    public static String getPathForUnoil(String officeInstall)
    {
        File unopkgdir = new File(officeInstall, UNOPACKAGEDIR);
        if(!unopkgdir.exists())
        {
            return null;
        }
        File scriptf = null;
        String[] listunopkg = unopkgdir.list();
        int size = listunopkg.length;
        for(int i=0; i<size; i++)
        {
            if (listunopkg[i].toLowerCase().indexOf(SCRIPTF)>-1)
            {
                scriptf = new File(unopkgdir, listunopkg[i]);
            }
        }
        if(scriptf != null)
        {
            File unoil = new File(scriptf, UNOILJAR);
            if(unoil.exists())
            {
                String path = unoil.getParent();
                path = path.substring(path.indexOf(UNOPACKAGEDIR));
                return officeInstall + path;
            }
        }
        return null;
    }

    /* public static void main(String[] args) {
        SVersionRCFile ov;
        Hashtable versions;

        if (args.length == 0)
            ov = new SVersionRCFile();
        else
            ov = new SVersionRCFile(args[0]);

        try {
            versions = ov.getVersions();
        }
        catch (IOException ioe) {
            System.err.println("Error getting versions: " + ioe.getMessage());
            return;
        }

        Enumeration enum = versions.keys();

        while (enum.hasMoreElements()) {
            String name = (String)enum.nextElement();
            System.out.println("Name: " + name + ", Path: " +
                (String)versions.get(name));
        }
    } */
}

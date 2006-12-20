/*
 * ProductDescription.java
 *
 * Created on November 28, 2005, 4:51 PM
 *
 */

package org.openoffice.setup.SetupData;

import org.openoffice.setup.InstallData;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
/**
 *
 * @author Christof Pintaske
 */
public class ProductDescription {

    private class Pair {
        public Pattern search;
        public String  replacement;

        public Pair(Pattern key, String value) {
            search      = key;
            replacement = value;
        }
    }

    private Vector macro;   /* macro list with precompiled regex patterns */
    private HashMap map;    /* conventional key-value pairs */
    private String backslashText = "THIS_IS_A_BACKSLASH";

    /**
     * read properties from package description data
     */

    protected ProductDescription(XMLPackageDescription descriptionData) {
        macro = new Vector();
        map   = new HashMap();
        parse(descriptionData);
     }

    /**
     * retrieve information about general installation properties
     */

    public void setNewMacro(String key, String value) {
        if ((key != null) && (value != null)) {
            String  match   = "\\$\\{" + key + "\\}";
            Pattern pattern = Pattern.compile(match, Pattern.CASE_INSENSITIVE);

            map.put(key, value);
            Iterator m = map.entrySet().iterator();
            // while ( m.hasNext() ) {
            //     Map.Entry entry = (Map.Entry) m.next();
            //     System.out.println( "MAP:" + entry.getKey() + ":" + entry.getValue() );
            // }

            // Does the pair with the same pattern already exist? Then it has to be removed
            for (int i = 0; i < macro.size(); i++) {
                Pair pair = (Pair) macro.get(i);
                if ( pair.search.pattern().equals(match) ) {
                    macro.remove(i);
                }
            }

            macro.add(new Pair(pattern, value));

            // Inserting new Pair at the beginning of the macro vector. Then it will
            // be used instead of older version during macro replacement.
            // For example ${DIR} in the select dir dialog, if this is set more than once.
            // macro.add(0, new Pair(pattern, value));
        }
    }

    public void dumpMacros() {
        for (int i = 0; i < macro.size(); i++) {
            Pair pair = (Pair) macro.get(i);
            System.out.println("Key: " + pair.search.pattern() + " Value: " + pair.replacement );
        }
    }

    public String get(String key) {
        return (String) map.get(key);
    }

    private boolean doMaskBackslash(String[] arr) {
        boolean changed = false;

        int index = arr[0].indexOf('\\');
        if ( index >= 0 ) {
            arr[0] = arr[0].replaceAll("\\", backslashText);
            // arr[0] = arr[0].replace("\\", backslashText);
            changed = true;
        }

        return changed;
    }

    private String doUnmaskBackslash(String s) {
        s = s.replaceAll(backslashText, "\\");
        // s = s.replace(backslashText, "\\");
        return s;
    }

    public String replaceMacros(String s) {

        String result = s;

        for (int i = 0; i < macro.size(); i++) {
            Pair pair = (Pair) macro.get(i);
            Pattern pattern = pair.search;

            Matcher matcher = pattern.matcher(result);

            String replace = pair.replacement;
            result = matcher.replaceAll(replace);

            // masquerading backslashes in String replace (important for Windows pathes)
            //  String[] arr1 = { replace };
            //  boolean masked = doMaskBackslash(arr1);
            //  result = matcher.replaceAll(arr1[0]);
            //  if (masked) {
            //      result = doUnmaskBackslash(result);
            //  }
        }

        return result;
    }

    /**
     * parse the wrapped package description
     */

    private void parse(XMLPackageDescription data) {

        XMLPackageDescription section;

        /* product description is a leaf at the root */
        if (!data.getKey().equals("product")) {
            section = data.getElement("product");
            if (section != null) {
                parse(section);
            }
        } else {
            InstallData installData = InstallData.getInstance();

            /* check for a default installation directory */
            section = data.getElement("defaultdir");
            if (section != null) {
                String value = section.getValue();
                if (value != null) {
                    installData.setDefaultDir(value);
                    installData.setInstallDir(value);
                }
            }

            /* check for the package format of this installation set */
            section = data.getElement("packageformat");
            if (section != null) {
                String value = section.getValue();
                if (value != null) {
                    installData.setPackageFormat(value);
                }
            }

            /* check for the package directory of this installation set */
           section = data.getElement("packagedirectory");
            if (section != null) {
                String value = section.getValue();
                if ((value != null) && (! value.equals(""))) {
                    installData.setPackageSubdir(value);
                }
            }

            /* check for any macro definitions */
            for (Enumeration e = data.elements(); e.hasMoreElements(); ) {
                XMLPackageDescription p = (XMLPackageDescription) e.nextElement();
                if (p.getKey().equals("macro")) {
                    String  key     = p.getAttribute("key");
                    String  value   = p.getValue();

                    if ((key != null) && (value != null)) {
                        String  match   = "\\$\\{" + key + "\\}";
                        Pattern pattern = Pattern.compile(match, Pattern.CASE_INSENSITIVE);

                        map.put(key, value);
                        macro.add(new Pair(pattern, value));
                    }
                }
            }
        }
    }

}
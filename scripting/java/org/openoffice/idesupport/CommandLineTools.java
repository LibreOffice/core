/*************************************************************************
 *
 *  $RCSfile: CommandLineTools.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-09-10 10:45:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Enumeration;
import java.util.StringTokenizer;

import com.sun.star.script.framework.browse.ScriptEntry;
import com.sun.star.script.framework.browse.ParcelDescriptor;

import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.idesupport.filter.AllFilesFilter;
import com.sun.star.script.framework.browse.XMLParserFactory;
import org.openoffice.idesupport.*;

public class CommandLineTools {
    private static final String PARCEL_XML_FILE =
        ParcelZipper.PARCEL_DESCRIPTOR_XML;

    public static void main(String[] args) {
        CommandLineTools driver = new CommandLineTools();
        Command command = driver.parseArgs(args);

        // Get the URL for the Office DTD directory and pass it to the
        // XMLParserFactory so that Office xml files can be parsed
        try {
            SVersionRCFile sv = SVersionRCFile.createInstance();
            String office = sv.getDefaultVersion().getPath();

            OfficeInstallation oi = new OfficeInstallation(office);
            String url = oi.getURL("share/dtd/officedocument/1_0/");
            XMLParserFactory.setOfficeDTDURL(url);
        }
        catch (IOException ioe) {
            System.err.println("Error getting Office DTD directory");
        }

        if (command == null)
            driver.printUsage();
        else {
            try {
                command.execute();
            }
            catch (Exception e) {
                driver.fatal("Error: " + e.getMessage());
            }
        }
    }

    private interface Command {
        public void execute() throws Exception;
    }

    private void printUsage() {
        System.out.println("java " + getClass().getName() + " -h " +
            "prints this message");
        System.out.println("java " + getClass().getName() + " -d " +
            "<script parcel zip file> " +
            "<destination document or directory>");
        System.out.println("java " + getClass().getName() + " -g " +
            "[parcel root directory] [options] [script names]");
        System.out.println("options:");
        System.out.println("\t[-l language[=supported extension 1[" +
            File.pathSeparator + "supported extension 2]]]");
        System.out.println("\t[-p name=value]");
        System.out.println("\t[-v]");
    }

    private void fatal(String message) {
        System.err.println(message);
        System.exit(-1);
    }

    private Command parseArgs(String[] args) {

        if (args.length < 1) {
            return null;
        }
        else if (args[0].equals("-h")) {
            return new Command() {
                public void execute() {
                    printUsage();
                }
            };
        }
        else if(args[0].equals("-d")) {
            if (args.length != 3)
                return null;
            else
                return new DeployCommand(args[1], args[2]);
        }
        else if(args[0].equals("-g")) {

            if (args.length == 1)
                return new GenerateCommand(System.getProperty("user.dir"));

            GenerateCommand command;
            int i = 1;
            if (!args[i].startsWith("-"))
                command = new GenerateCommand(args[i++]);
            else
                command = new GenerateCommand(System.getProperty("user.dir"));

            for (; i < args.length; i++) {
                if (args[i].equals("-l")) {
                    command.setLanguage(args[++i]);
                }
                else if (args[i].equals("-p")) {
                    command.addProperty(args[++i]);
                }
                else if (args[i].equals("-v")) {
                    command.setVerbose();
                }
                else {
                    command.addScript(args[i]);
                }
            }
            return command;
        }
        return null;
    }

    private static class GenerateCommand implements Command {

        private File basedir, contents, parcelxml;
        private boolean verbose = false;
        private String language = null;
        private MethodFinder finder = null;
        private ArrayList scripts = null;
        private Hashtable properties = new Hashtable(3);

        public GenerateCommand(String basedir) {
            this.basedir = new File(basedir);
            this.contents = new File(basedir, ParcelZipper.CONTENTS_DIRNAME);
            this.parcelxml = new File(contents, PARCEL_XML_FILE);
        }

        public void setLanguage(String language) {
            StringTokenizer tokenizer = new StringTokenizer(language, "=");
            this.language = tokenizer.nextToken();

            if (this.language.toLowerCase().equals("java")) {
                this.finder = JavaFinder.getInstance();
                return;
            }

            if (tokenizer.hasMoreTokens()) {
                String ext = (String)tokenizer.nextToken();
                String[] extensions;

                if (ext.indexOf(File.pathSeparator) != -1) {
                    tokenizer = new StringTokenizer(ext, File.pathSeparator);
                    extensions = new String[tokenizer.countTokens()];
                    int i = 0;

                    while(tokenizer.hasMoreTokens())
                        extensions[i++] = (String)tokenizer.nextToken();
                }
                else {
                    extensions = new String[1];
                    extensions[0] = ext;
                }
                this.finder = new ExtensionFinder(this.language, extensions);
            }
        }

        public void addProperty(String prop) {
            StringTokenizer tok = new StringTokenizer(prop, "=");

            if (tok.countTokens() != 2)
                return;

            String name = tok.nextToken();
            String value = tok.nextToken();

            properties.put(name, value);
        }

        public void setVerbose() {
            verbose = true;
        }

        public void addScript(String script) {
            if (language == null)
                return;

            addScript(new ScriptEntry(language, script, script, basedir.getName()));
        }

        public void addScript(ScriptEntry entry) {
            if (scripts == null)
                scripts = new ArrayList(3);
            scripts.add(entry);
        }

        public void execute() throws Exception {

            if (basedir.isDirectory() != true) {
                throw new Exception(basedir.getName() + " is not a directory");
            }
            else if (contents.exists() != true) {
                throw new Exception(basedir.getName() +
                    " does not contain a Contents directory");
            }

            if (language == null && parcelxml.exists() == false) {
                throw new Exception(parcelxml.getName() + " not found and language " +
                    "not specified");
            }

            if (language != null && parcelxml.exists() == true) {
                ParcelDescriptor desc;
                String desclang = "";

                desc = new ParcelDescriptor(parcelxml);
                desclang = desc.getLanguage().toLowerCase();

                if (!desclang.equals(language.toLowerCase()))
                    throw new Exception(parcelxml.getName() + " already exists, " +
                        "and has a different language attribute: " +
                        desc.getLanguage());
            }

            if (language != null && scripts == null) {
                if (finder == null)
                    throw new Exception("Extension list not specified for this language");

                log("Searching for " + language + " scripts");

                ScriptEntry[] entries = finder.findMethods(contents);
                for (int i = 0; i < entries.length; i++) {
                    addScript(entries[i]);
                    log("Found: " + entries[i].getLogicalName());
                }
            }

            if (scripts != null) {
                if (scripts.size() == 0)
                    throw new Exception("No valid scripts found");

                ParcelDescriptor desc = new ParcelDescriptor(parcelxml, language);
                desc.setScriptEntries((ScriptEntry[])scripts.toArray(new ScriptEntry[0]));
                if (properties.size() != 0) {
                    Enumeration enum = properties.keys();

                    while (enum.hasMoreElements()) {
                        String name = (String)enum.nextElement();
                        String value = (String)properties.get(name);
                        log("Setting property: " +  name + " to " + value);

                        desc.setLanguageProperty(name, value);
                    }
                }
                desc.write();
            }
            else {
                if (parcelxml.exists() == false)
                    throw new Exception("No valid scripts found");
            }

            contents = new File(contents.getAbsolutePath());
            String name = ParcelZipper.getParcelZipper().zipParcel(contents, AllFilesFilter.getInstance());
            System.out.println(name + " generated");
        }

        private void log(String message) {
            if (verbose)
                System.out.println(message);
        }
    }

    private static class DeployCommand implements Command {

        File source, target;

        public DeployCommand(String source, String target) {
            this.source = new File(source);
            this.target = new File(target);
        }

        public void execute() throws Exception {
            ParcelZipper.getParcelZipper().deployParcel(source, target);
            System.out.println(source.getName() +
                " successfully deployed to " + target.getAbsolutePath());
        }
    }
}

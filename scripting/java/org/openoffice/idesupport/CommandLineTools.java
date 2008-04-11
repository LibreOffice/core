/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CommandLineTools.java,v $
 * $Revision: 1.11 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Enumeration;
import java.util.StringTokenizer;

import com.sun.star.script.framework.container.ScriptEntry;
import com.sun.star.script.framework.container.ParcelDescriptor;

import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.idesupport.filter.AllFilesFilter;
import com.sun.star.script.framework.container.XMLParserFactory;
import org.openoffice.idesupport.*;

public class CommandLineTools {
    private static final String PARCEL_XML_FILE =
        ParcelZipper.PARCEL_DESCRIPTOR_XML;

    private static String officePath = null;

    public static void main(String[] args) {
        CommandLineTools driver = new CommandLineTools();
        Command command = driver.parseArgs(args);

        // Get the URL for the Office DTD directory and pass it to the
        // XMLParserFactory so that Office xml files can be parsed
        if (officePath == null)
        {
            try {
                SVersionRCFile sv = SVersionRCFile.createInstance();
                if (sv.getDefaultVersion() != null)
                {
                    officePath = sv.getDefaultVersion().getPath();
                }
            }
            catch (IOException ioe) {
                System.err.println("Error getting Office directory");
            }
        }

        if (officePath == null)
        {
            driver.fatalUsage("Error: Office Installation path not set");
        }

        File officeDir = new File(officePath);
        if (officeDir.exists() == false || officeDir.isDirectory() == false)
        {
            driver.fatalUsage(
                "Error: Office Installation path not valid: " + officePath);
        }

        OfficeInstallation oi = new OfficeInstallation(officePath);
        String url = oi.getURL("share/dtd/officedocument/1_0/");
        XMLParserFactory.setOfficeDTDURL(url);

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
        System.out.println("java " + getClass().getName() +
            " [-o Path to Office Installation] " +
            "-d <script parcel zip file> " +
            "<destination document or directory>");
        System.out.println("java " + getClass().getName() +
            " [-o Path to Office Installation] " +
            "-g [parcel root directory] [options] [script names]");
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

    private void fatalUsage(String message) {
        System.err.println(message);
        System.err.println();
        printUsage();
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

        int i = 0;

        if(args[0].equals("-o")) {
            officePath = args[i+1];
            i += 2;
        }

        if(args[i].equals("-d")) {
            if ((args.length - i) != 3)
                return null;
            else
                return new DeployCommand(args[i+1], args[i+2]);
        }
        else if(args[i].equals("-g")) {

            if ((args.length - i) == 1)
                return new GenerateCommand(System.getProperty("user.dir"));

            GenerateCommand command;
            i++;
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
                    Enumeration enumer = properties.keys();

                    while (enumer.hasMoreElements()) {
                        String name = (String)enumer.nextElement();
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

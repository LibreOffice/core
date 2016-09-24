/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.libreoffice.example.java_scripts;

import java.io.*;
import java.util.Vector;


public class SubscribedNewsgroups {


    private static NewsGroup[] allSubscribed = null;
    private static boolean windows = false;

    public static void main(String[] args) {
        // Test the class
        SubscribedNewsgroups subscribed = new SubscribedNewsgroups();

        NewsGroup allGroups[] = subscribed.getNewsGroups();

        if (allGroups == null) {
            System.out.println("Could not find subscribed newsgroups from mozilla/netscape mailrc files");
        } else {
            for (int i = 0; i < allGroups.length; i++) {
                System.out.println("Hostname is: " + allGroups[i].getHostName() +
                                   " Newsgroup is: " + allGroups[i].getNewsgroupName());
            }
        }
    }



    // Only public method of the class
    // Returns and array of unique NewsGroup objects
    public NewsGroup[] getNewsGroups() {
        windows = false;

        if (System.getProperty("os.name").indexOf("Windows") != -1) {
            windows = true;
        }

        String mozillaHome = "";

        if (windows) {
            mozillaHome = System.getProperty("user.home") +
                          System.getProperty("file.separator") + "Application Data" +
                          System.getProperty("file.separator") + "Mozilla" +
                          System.getProperty("file.separator") + "Profiles";
        } else {
            mozillaHome = System.getProperty("user.home") +
                          System.getProperty("file.separator") + ".mozilla";
        }

        if (!new File(mozillaHome).isDirectory()) {
            return null;
        }

        // Get all the profiles belonging to the user
        File profiles[] = findProfiles(new File(mozillaHome));

        if (profiles.length < 1) {
            return null;
        }

        // Get the News directory for each profile
        File allNewsDirs[] = new File[ profiles.length ];

        for (int i = 0; i < profiles.length; i++) {
            File newsDir = findNewsDir(profiles[i]);
            allNewsDirs[i] = newsDir;
        }

        // Check that at least one News directory exists and remove nulls
        boolean newsFound = false;

        for (int i = 0; i < allNewsDirs.length; i++) {
            if (allNewsDirs[i] != null) {
                newsFound = true;
                break;
            }
        }

        if (!newsFound) {
            return null;
        }

        // Get all the mailrc files for each News directory
        File allMailrcs[] = findMailrcFiles(allNewsDirs);

        if (allMailrcs == null) {
            return null;
        }

        ArrayList<NewsGroup> subscribed = new ArrayList<NewsGroup>();

        // Get the newsgroups in each mailrc file
        for (int i = 0; i < allMailrcs.length; i++) {
            File mailrc = (File) allMailrcs[i];
            NewsGroup newsgroup[] = findNewsgroups(mailrc);

            //if the Newsgroup has not already been added to the list
            for (int j = 0; j < newsgroup.length; j++) {
                // if newsgroup is unique then add to the list
                if (!listed(newsgroup[j], subscribed)) {
                    subscribed.addElement(newsgroup[j]);
                }
            }
        }

        // Copy all unique Newsgroups into the global array
        allSubscribed = new NewsGroup[ subscribed.size() ];
        subscribed.toArray(allSubscribed);

        // Test that at least one subscribed newsgroup has been found
        if (allSubscribed.length < 1) {
            return null;
        }

        return allSubscribed;
    }




    // Tests if the NewsGroup object has already been listed by another mailrc file
    private static boolean listed(NewsGroup newsgroup,
                                  ArrayList<NewsGroup> uniqueSubscription) {
        for (int i = 0; i < uniqueSubscription.size(); i++) {
            NewsGroup tempGroup = uniqueSubscription.elementAt(i);

            // Test for duplication
            if (newsgroup.getHostName().equalsIgnoreCase(tempGroup.getHostName()) &&
                newsgroup.getNewsgroupName().equalsIgnoreCase(tempGroup.getNewsgroupName()))
                return true;
        }

        return false;
    }




    // Finds all the NewsGroups in an individual mailrc file
    private static NewsGroup[] findNewsgroups(File mailrcfile) {

        String hostname = "";
        String newsgroup = "";
        NewsGroup mailrcNewsGroups[] = null;

        //Retrieve name of news host/server from file name
        //Sequentially access each of the newsgroups
        //If the newsgroup is not already contained in the global NewsGroup[] array then add it

        String filename = mailrcfile.getPath();

        if (windows) {
            // Windows format "staroffice-news.germany.sun.com.rc"
            int hostNameStart = filename.lastIndexOf("\\") + 1;
            int hostNameEnd = filename.indexOf(".rc");
            hostname = filename.substring(hostNameStart, hostNameEnd);
        } else {
            // Unix/Linux format "newsrc-staroffice-news.germany.sun.com"
            int hostNameStart = filename.lastIndexOf("newsrc-") + 7;
            hostname = filename.substring(hostNameStart, filename.length());
        }

        // Assumes the content format in Window is the same as Unix/Linux (unknown at the moment)
        // i.e. a list of newsgroups each ending with a ":"
        LineNumberReader in = null;

        try {
            in = new LineNumberReader(new FileReader(mailrcfile));
            ArrayList groups = new ArrayList();
            String inString = "";
            int line = 0;

            while (inString != null) {
                in.setLineNumber(line);
                inString = in.readLine();
                line++;

                if (inString != null) {
                    int newsgroupEnd = inString.indexOf(":");
                    newsgroup = inString.substring(0, newsgroupEnd);
                    NewsGroup group = new NewsGroup(hostname, newsgroup);
                    groups.addElement(group);
                }
            }

            mailrcNewsGroups = new NewsGroup[ groups.size() ];
            groups.copyInto(mailrcNewsGroups);
            in.close();
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }

        return mailrcNewsGroups;
    }


    // Finds all the mailrc files for all the given News directories
    private static File[] findMailrcFiles(File[] newsDirs) {
        ArrayList allFiles = new ArrayList();

        for (int i = 0; i < newsDirs.length; i++) {
            if (newsDirs[i] != null) {
                File mailrcFiles[] = newsDirs[i].listFiles(new VersionFilter());

                if (mailrcFiles != null) {
                    for (int j = 0; j < mailrcFiles.length; j++) {
                        allFiles.addElement(mailrcFiles[j]);
                    }
                }
            }
        }

        File allMailrcFiles[] = new File[ allFiles.size() ];
        allFiles.copyInto(allMailrcFiles);

        if (allMailrcFiles.length == 0) {
            return null;
        }

        return allMailrcFiles;
    }


    // Finds all profiles belonging to one user (can be more than one)
    private static File[] findProfiles(File start) {
        // Get all files and directories in .mozilla
        File allFiles[] = start.listFiles();
        File[] dirs = new File[allFiles.length];
        int dirCounter = 0;

        // Remove files leaving directories only
        for (int i = 0; i < allFiles.length; i++) {
            if (allFiles[i].isDirectory()) {
                dirs[dirCounter] = allFiles[i];
                dirCounter++;
            }
        }

        // Add each directory to a user profile array
        File[] profileDirs = new File[dirCounter];

        for (int i = 0; i < dirCounter; i++) {
            profileDirs[i] = dirs[i];
        }

        // return a File array containing the profile dirs
        return profileDirs;
    }


    // Recursively searches for the News directory for a given profile directory
    private static File findNewsDir(File start) {
        File mailrcFile = null;

        // File array containing all matches for the version filter ("News")
        File files[] = start.listFiles(new VersionFilter());

        // If the array is empty then no matches were found
        if (files.length == 0) {
            // File array of all the directories in File start
            File dirs[] = start.listFiles(new DirFilter());

            // for each of the directories check for a match
            for (int i = 0; i < dirs.length; i++) {
                mailrcFile = findNewsDir(dirs[i]);

                if (mailrcFile != null) {
                    // break the for loop
                    break;
                }
            }
        } else {
            // end recursion
            // Check for a News directory inside the News directory (fix for bug)
            // Original solution had only "mailrcFile = files[0];"

            boolean noChildNews = true;
            File checkChildNewsDirs[] = files[0].listFiles(new VersionFilter());

            if (checkChildNewsDirs != null) {
                for (int i = 0; i < checkChildNewsDirs.length; i++) {
                    if (checkChildNewsDirs[i].getName().equals("News")) {
                        noChildNews = false;
                        break;
                    }
                }
            }

            if (noChildNews) {
                mailrcFile = files[0];
            } else {
                String childNewsPathName = files[0].getAbsolutePath() +
                                           System.getProperty("file.separator") + "News";
                mailrcFile = new File(childNewsPathName);
            }

        }

        // return a File representing the News dir in a profile
        return mailrcFile;
    }
}



class DirFilter implements FileFilter {
    public boolean accept(File aFile) {
        return aFile.isDirectory();
    }
}


class VersionFilter implements FileFilter {
    public boolean accept(File aFile) {
        if (System.getProperty("os.name").indexOf("Windows") != -1) {
            if (aFile.getName().compareToIgnoreCase("News") == 0 ||
                aFile.getName().indexOf(".rc") != -1) {
                return true;
            }
        } else {
            if (aFile.getName().compareToIgnoreCase("News") == 0 ||
                aFile.getName().indexOf("newsrc") != -1) {
                return true;
            }
        }

        return false;
    }
}

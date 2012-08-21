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

import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JEditorPane;
import javax.swing.JButton;
import java.net.URL;
import javax.swing.event.HyperlinkListener;
import javax.swing.event.HyperlinkEvent;
import java.net.MalformedURLException;
import java.io.File;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.event.ActionListener;
import java.util.LinkedList;

class HelpWindow
    implements ActionListener
{
    public static synchronized HelpWindow Instance ()
    {
        if (maInstance == null)
            maInstance = new HelpWindow();
        return maInstance;
    }

    public void loadFile (String sFilename)
    {
        File aFile = new File (sFilename);
        try
        {
            loadURL (aFile.toURI().toURL());
        }
        catch (MalformedURLException e)
        {
            e.printStackTrace (System.err);
        }
    }
    public void loadURL (String sURL)
    {
        try
        {
            loadURL (new URL (sURL));
        }
        catch (MalformedURLException e)
        {
            e.printStackTrace (System.err);
        }
    }




    public void loadURL (URL aURL)
    {
        maHistory.addLast (aURL);
        selectHistoryPage (maHistory.size()-1);
        maFrame.toFront ();
    }




    private HelpWindow ()
    {
        try
        {
            maCurrentHistoryEntry = -1;
            maHistory = new LinkedList();

            maFrame = new JFrame ();
            maFrame.addWindowListener (new WindowAdapter ()
                {
                    public void windowClosing (WindowEvent e)
                    {
                        maInstance = null;
                    }
                });
            maContent = createContentWidget();

            maFrame.getContentPane().setLayout (new GridBagLayout());
            GridBagConstraints aConstraints = new GridBagConstraints ();
            aConstraints.gridx = 0;
            aConstraints.gridy = 0;
            aConstraints.gridwidth = 3;
            aConstraints.weightx = 1;
            aConstraints.weighty = 1;
            aConstraints.fill = GridBagConstraints.BOTH;
            maFrame.getContentPane().add (new JScrollPane (maContent), aConstraints);

            aConstraints = new GridBagConstraints();
            aConstraints.gridx = 0;
            aConstraints.gridy = 1;
            maPrevButton = new JButton ("Prev");
            maFrame.getContentPane().add (maPrevButton, aConstraints);
            maPrevButton.addActionListener (this);

            aConstraints = new GridBagConstraints();
            aConstraints.gridx = 1;
            aConstraints.gridy = 1;
            maNextButton = new JButton ("Next");
            maFrame.getContentPane().add (maNextButton, aConstraints);
            maNextButton.addActionListener (this);

            aConstraints = new GridBagConstraints();
            aConstraints.gridx = 2;
            aConstraints.gridy = 1;
            aConstraints.anchor = GridBagConstraints.EAST;
            JButton aButton = new JButton ("Close");
            maFrame.getContentPane().add (aButton, aConstraints);
            aButton.addActionListener (this);

            maFrame.setSize (600,400);
            maFrame.setVisible (true);
        }
        catch (Exception e)
        {}
    }

    public void actionPerformed (java.awt.event.ActionEvent e)
    {
        if (e.getActionCommand().equals("Prev"))
        {
            selectHistoryPage (maCurrentHistoryEntry - 1);
        }
        else if (e.getActionCommand().equals("Next"))
        {
            selectHistoryPage (maCurrentHistoryEntry + 1);
        }
        else if (e.getActionCommand().equals("Close"))
        {
            maFrame.dispose ();
            maInstance = null;
        }
    }

    private JEditorPane createContentWidget ()
    {
        JEditorPane aContent = new JEditorPane ();
        aContent.setEditable (false);
        aContent.addHyperlinkListener (new HyperlinkListener()
            {
                public void hyperlinkUpdate (HyperlinkEvent e)
                {
                    if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
                        HelpWindow.Instance().loadURL (e.getURL());
                }
            });
        return aContent;
    }

    private void selectHistoryPage (int i)
    {
        if (i < 0)
            i = 0;
        else if (i >= maHistory.size()-1)
            i = maHistory.size()-1;
        if (i != maCurrentHistoryEntry)
        {
            URL aURL = (URL)maHistory.get (i);
            try
            {
                maContent.setPage (aURL);
            }
            catch (java.io.IOException ex)
            {
                ex.printStackTrace(System.err);
            }

            maCurrentHistoryEntry = i;
        }

        maPrevButton.setEnabled (maCurrentHistoryEntry > 0);
        maNextButton.setEnabled (maCurrentHistoryEntry < maHistory.size()-1);
    }

    private static HelpWindow maInstance = null;
    private JFrame maFrame;
    private JEditorPane maContent;
    private LinkedList maHistory;
    private int maCurrentHistoryEntry;
    private JButton maPrevButton;
    private JButton maNextButton;
}

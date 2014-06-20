/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/

package org.apache.openoffice.ooxml.viewer;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.util.Vector;

import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTabbedPane;
import javax.swing.UIManager;

import org.apache.openoffice.ooxml.framework.part.OOXMLPackage;
import org.apache.openoffice.ooxml.framework.part.parser.ParserFactory;
import org.apache.openoffice.ooxml.parser.Log;
import org.apache.openoffice.ooxml.viewer.content.ContentView;

/** A simple viewer for the streams inside an OOXML file.
 */
public class OOXMLViewer
{
    public static void main (final String[] aArguments)
    {
        try
        {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        Log.Dbg = null;

        // Process options.
        final Vector<String> aFilenames = new Vector<>();
        for (int nIndex=0; nIndex<aArguments.length; ++nIndex)
        {
            if (aArguments[nIndex].startsWith("-"))
            {
                switch (aArguments[nIndex])
                {
                    case "-t":
                        ++nIndex;
                        if (nIndex >= aArguments.length)
                        {
                            System.err.printf("expecting argument after option '-t'");
                            System.exit(1);
                        }
                        ParserFactory.SetParserTableFilename(aArguments[nIndex]);
                        break;

                    default:
                        System.out.printf("unknown option '%s'\n", aArguments[nIndex]);
                        System.exit(1);;
                        break;

                }
            }
            else
                aFilenames.add(aArguments[nIndex]);
        }

        for (final String sFilename : aFilenames)
        {
            final OOXMLViewer aViewer = new OOXMLViewer();
            aViewer.SetFile(new File(sFilename));
            aViewer.maFrame.setVisible(true);
        }
    }




    private OOXMLViewer ()
    {
        maFrame = new JFrame("OOXML Viewer");
        maFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        maFrame.setSize(1024,768);
        maContainer = new JPanel();
        maFrame.add(maContainer, BorderLayout.CENTER);

        maFrame.setJMenuBar(CreateMenuBar());
        maFrame.addKeyListener(new KeyListener());
    }




    private void SetFile (final File aFile)
    {
        if ( ! aFile.exists())
        {
            JOptionPane.showMessageDialog(
                    maFrame,
                    "File '"+aFile.toString()+"' does not exist",
                    "File Error",
                    JOptionPane.ERROR_MESSAGE);
        }
        else if ( ! aFile.canRead())
        {
            JOptionPane.showMessageDialog(
                    maFrame,
                    "Can not open '"+aFile.toString()+"' for reading",
                    "File Error",
                    JOptionPane.ERROR_MESSAGE);
        }
        else
        {
            maContainer.removeAll();
            maContainer.setLayout(new BorderLayout());

            final OOXMLPackage aPackage = OOXMLPackage.Create(aFile);
            final JScrollPane aDetailViewContainer = new JScrollPane(
                JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
            aDetailViewContainer.getVerticalScrollBar().setUnitIncrement(10);
            final DetailViewManager aDetailViewManager = new DetailViewManager(aDetailViewContainer, aPackage);

            final JTabbedPane aLeftSidebar = new JTabbedPane();

            final ContentView aContentView = new ContentView(aDetailViewManager, aPackage);
            aLeftSidebar.addTab("Content", new JScrollPane(aContentView));

            final StreamView aFragmentView = new StreamView(aDetailViewManager, aPackage);
            aLeftSidebar.addTab("Streams", new JScrollPane(aFragmentView));

            final PartsView aPartsView = new PartsView(aDetailViewManager, aPackage);
            aLeftSidebar.addTab("Parts", new JScrollPane(aPartsView));

            final JSplitPane aPane = new JSplitPane(
                    JSplitPane.HORIZONTAL_SPLIT,
                    aLeftSidebar,
                    aDetailViewContainer
                    );
            aPane.setDividerLocation(200);

            maContainer.add(aPane, BorderLayout.CENTER);

            aFragmentView.ShowInitialPart();
        }
    }




    private JMenuBar CreateMenuBar ()
    {
        final JMenuBar aMenuBar = new JMenuBar();

        final JMenu aFileMenu = new JMenu("File");
        aMenuBar.add(aFileMenu);

        final JMenuItem aOpenItem = new JMenuItem("Open");
        aOpenItem.addActionListener(new ActionListener()
        {
            @Override public void actionPerformed (final ActionEvent aEvent)
            {
                final JFileChooser aFileChooser = new JFileChooser();
                final int nResult = aFileChooser.showOpenDialog(null);
                if (nResult == JFileChooser.APPROVE_OPTION)
                {
                    final OOXMLViewer aViewer = new OOXMLViewer();
                    aViewer.SetFile(aFileChooser.getSelectedFile());
                }
            }
        });
        aFileMenu.add(aOpenItem);

        return aMenuBar;
    }




    private final JFrame maFrame;
    private final JComponent maContainer;
}

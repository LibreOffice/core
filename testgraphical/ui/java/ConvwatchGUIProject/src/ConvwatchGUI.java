
import java.awt.Dimension;
import java.awt.Image;
import java.io.File;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.SwingWorker;

/*
**************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
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
**************************************************************************
 */

/*
 * Simple windows, which should show differences if there are some
 */

public class ConvwatchGUI extends javax.swing.JFrame
{
    private ImageIcon[] m_aImageIcon;
    private String m_sInifile;
    private int m_nMaxPages;
    private int m_nCurrentPage;

    /** Creates new form ConvwatchGUI
     * @param args
     */

    private ConvwatchGUI(String args[])
    {
        if (args.length > 0)
        {
            if (args[0].endsWith(".ini"))
            {
                m_sInifile = args[0];
                fillImageIconsFromInifile();
            }
            else
            {
                fillImageIcons(args);
            }
        }

        String sVersion = System.getProperty("java.version");
        Float f = Float.valueOf(sVersion.substring(0,3));
        if (f.floatValue() < (float)1.6)
        {
            System.out.println("You need at least Java version 1.6");
            System.exit(1);
        }

        initComponents();
        jLabelDocumentName.setText("Document: " + m_sInifile);
    }

    void fillImageIconsFromInifile()
    {
        File aFile = new File(m_sInifile);
        if (!aFile.exists())
        {
            GlobalLogWriter.println("Inifile '" + m_sInifile + "' not found.");
            printUsage();
            System.exit(1);
        }

        IniFile aIniFile = new IniFile(aFile);
        int nPages = aIniFile.getIntValue("global", "pages", 0);
        if (nPages < 1)
        {
            System.out.println("No pages found.");
        }
        m_nMaxPages = nPages;
        m_nCurrentPage = 1;
        fillImageIcons();
    }

    private void fillImageIcons()
    {
        File aFile = new File(m_sInifile);
        IniFile aIniFile = new IniFile(aFile);
        String sSection = "page" + m_nCurrentPage;
        String[] files = new String[3];
        files[0] = aIniFile.getValue(sSection, "newgfx"); // current created picture
        files[1] = aIniFile.getValue(sSection, "oldgfx"); // reference picture
        files[2] = aIniFile.getValue(sSection, "diffgfx");
        fillImageIcons(files);
    }

    /**
     * Give 3 file names
     * @param args
     */
    private void fillImageIcons(String args[])
    {
        boolean bLoadImages = false;
        m_aImageIcon = new ImageIcon[3];
        for (int i=0;i<3;i++)
        {
            if (args.length > i && args[i] != null)
            {
                File aFile = new File(args[i]);
                if (aFile.exists())
                {
                    // TODO: Load images
                    // Image aImage = new BufferedImage(100,100, BufferedImage.TYPE_INT_RGB);
                    // aImage.
                    m_aImageIcon[i] = new ImageIcon(args[i]);
                    if (m_aImageIcon[i] != null)
                    {
                        bLoadImages = true;
                    }
                }
                else
                {
                    System.out.println("Can't read file: " + aFile.getName());
                    bLoadImages = false;
                }
            }
            else
            {
                System.out.println("There is no #" + (i + 1) + " image given.");
                bLoadImages = false;
            }
        }
//        if (!bLoadImages)
//        {
//            printUsage();
//            System.exit(1);
//        }



        // TODO: Set images.

        // formComponentResized(null);
    }

    private void printUsage()
    {
        System.out.println("Usage:");
        System.out.println("   ConvwatchGUI <pic1> <pic2> <pic3>");
        System.out.println("or ConvwatchGUI <inifile>");
    }

//    private int m_nOldWidth;

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanelOriginal = new javax.swing.JPanel();
        jLabelOriginalImage = new javax.swing.JLabel();
        jPanelReference = new javax.swing.JPanel();
        jLabelReferenceImage = new javax.swing.JLabel();
        jPanelDifference = new javax.swing.JPanel();
        jLabelDifferenceImage = new javax.swing.JLabel();
        jPanel4 = new javax.swing.JPanel();
        jButton1 = new javax.swing.JButton();
        jButton2 = new javax.swing.JButton();
        jButton3 = new javax.swing.JButton();
        jLabelCurrentPage = new javax.swing.JLabel();
        jPanel5 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jLabelDocumentName = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        addComponentListener(new java.awt.event.ComponentAdapter() {
            public void componentResized(java.awt.event.ComponentEvent evt) {
                formComponentResized(evt);
            }
        });
        addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                formPropertyChange(evt);
            }
        });

        jPanelOriginal.setBorder(javax.swing.BorderFactory.createTitledBorder("Picture"));

        javax.swing.GroupLayout jPanelOriginalLayout = new javax.swing.GroupLayout(jPanelOriginal);
        jPanelOriginal.setLayout(jPanelOriginalLayout);
        jPanelOriginalLayout.setHorizontalGroup(
            jPanelOriginalLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelOriginalLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabelOriginalImage, javax.swing.GroupLayout.DEFAULT_SIZE, 299, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanelOriginalLayout.setVerticalGroup(
            jPanelOriginalLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelOriginalLayout.createSequentialGroup()
                .addComponent(jLabelOriginalImage, javax.swing.GroupLayout.DEFAULT_SIZE, 514, Short.MAX_VALUE)
                .addContainerGap())
        );

        jPanelReference.setBorder(javax.swing.BorderFactory.createTitledBorder("Reference Picture"));

        javax.swing.GroupLayout jPanelReferenceLayout = new javax.swing.GroupLayout(jPanelReference);
        jPanelReference.setLayout(jPanelReferenceLayout);
        jPanelReferenceLayout.setHorizontalGroup(
            jPanelReferenceLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelReferenceLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabelReferenceImage, javax.swing.GroupLayout.DEFAULT_SIZE, 299, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanelReferenceLayout.setVerticalGroup(
            jPanelReferenceLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelReferenceLayout.createSequentialGroup()
                .addComponent(jLabelReferenceImage, javax.swing.GroupLayout.DEFAULT_SIZE, 514, Short.MAX_VALUE)
                .addContainerGap())
        );

        jPanelDifference.setBorder(javax.swing.BorderFactory.createTitledBorder("Difference"));

        jLabelDifferenceImage.setName("DifferenceImage"); // NOI18N

        javax.swing.GroupLayout jPanelDifferenceLayout = new javax.swing.GroupLayout(jPanelDifference);
        jPanelDifference.setLayout(jPanelDifferenceLayout);
        jPanelDifferenceLayout.setHorizontalGroup(
            jPanelDifferenceLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelDifferenceLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabelDifferenceImage, javax.swing.GroupLayout.DEFAULT_SIZE, 298, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanelDifferenceLayout.setVerticalGroup(
            jPanelDifferenceLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanelDifferenceLayout.createSequentialGroup()
                .addComponent(jLabelDifferenceImage, javax.swing.GroupLayout.DEFAULT_SIZE, 514, Short.MAX_VALUE)
                .addContainerGap())
        );

        jPanel4.setBorder(javax.swing.BorderFactory.createTitledBorder("Action"));

        jButton1.setText("Close");
        jButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton1ActionPerformed(evt);
            }
        });

        jButton2.setLabel("prev page");
        jButton2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton2ActionPerformed(evt);
            }
        });

        jButton3.setLabel("next page");
        jButton3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton3ActionPerformed(evt);
            }
        });

        jLabelCurrentPage.setText("Current page: 1");

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jButton2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jButton3)
                .addGap(18, 18, 18)
                .addComponent(jLabelCurrentPage, javax.swing.GroupLayout.PREFERRED_SIZE, 107, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 614, Short.MAX_VALUE)
                .addComponent(jButton1)
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel4Layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jButton1)
                    .addComponent(jButton2)
                    .addComponent(jButton3)
                    .addComponent(jLabelCurrentPage))
                .addContainerGap())
        );

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder("Information"));

        jLabel1.setText("<html>Here you see a graphical compare by pictures created with a current running office, a stored reference picture and the difference between those both pictures created by ImageMagicks 'composite'.\n</html>"); // NOI18N

        jLabelDocumentName.setText("jLabel2");

        javax.swing.GroupLayout jPanel5Layout = new javax.swing.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jLabel1, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 972, Short.MAX_VALUE)
                    .addComponent(jLabelDocumentName, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 972, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 50, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabelDocumentName, javax.swing.GroupLayout.PREFERRED_SIZE, 19, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jPanel5, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel4, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(jPanelOriginal, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jPanelReference, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jPanelDifference, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel5, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jPanelReference, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanelOriginal, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanelDifference, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private boolean bAdd = false;

    private void formComponentResized(java.awt.event.ComponentEvent evt)//GEN-FIRST:event_formComponentResized
    {//GEN-HEADEREND:event_formComponentResized
        // TODO add your handling code here:
        // we need to set icons to null
        // if we don't do this, icons can only grow, but not shrink :-(

        initialiseImages();
    }
    private void initialiseImages()
    {
        if (jLabelOriginalImage.getIcon() != null)
        {
            jLabelOriginalImage.setIcon(null);
            jLabelReferenceImage.setIcon(null);
            jLabelDifferenceImage.setIcon(null);

            int w = getWidth();
            int h = getHeight();
            if (bAdd)
            {
                this.setSize(w, h + 1);
                bAdd = false;
            }
            else
            {
                this.setSize(w, h - 1);
                bAdd = true;
            }
        }
        else
        {
            new ResizeImage(jLabelOriginalImage, m_aImageIcon[0]).execute();
            new ResizeImage(jLabelReferenceImage, m_aImageIcon[1]).execute();
            new ResizeImage(jLabelDifferenceImage, m_aImageIcon[2]).execute();
        }
        int dummy=0;
    }//GEN-LAST:event_formComponentResized

    private void jButton1ActionPerformed(java.awt.event.ActionEvent evt)//GEN-FIRST:event_jButton1ActionPerformed
    {//GEN-HEADEREND:event_jButton1ActionPerformed
        // TODO add your handling code here:
        System.exit(1);
    }//GEN-LAST:event_jButton1ActionPerformed

    private void formPropertyChange(java.beans.PropertyChangeEvent evt)//GEN-FIRST:event_formPropertyChange
    {//GEN-HEADEREND:event_formPropertyChange
        // TODO add your handling code here:
        int dummy = 0;
    }//GEN-LAST:event_formPropertyChange

    private void jButton2ActionPerformed(java.awt.event.ActionEvent evt)//GEN-FIRST:event_jButton2ActionPerformed
    {//GEN-HEADEREND:event_jButton2ActionPerformed
        // TODO add your handling code here:
        int nOldPage = m_nCurrentPage;
        if (m_nCurrentPage > 1)
        {
            m_nCurrentPage--;
        }
        if (nOldPage != m_nCurrentPage)
        {
            jLabelCurrentPage.setText("Current page: " + m_nCurrentPage);
            fillImageIcons();
            initialiseImages();
        }
    }//GEN-LAST:event_jButton2ActionPerformed

    private void jButton3ActionPerformed(java.awt.event.ActionEvent evt)//GEN-FIRST:event_jButton3ActionPerformed
    {//GEN-HEADEREND:event_jButton3ActionPerformed
        // TODO add your handling code here:
        int nOldPage = m_nCurrentPage;
        if (m_nCurrentPage < m_nMaxPages)
        {
            m_nCurrentPage++;
        }
        if (nOldPage != m_nCurrentPage)
        {
            jLabelCurrentPage.setText("Current page: " + m_nCurrentPage);
            fillImageIcons();
            initialiseImages();
        }
    }//GEN-LAST:event_jButton3ActionPerformed

    class ResizeImage extends SwingWorker <ImageIcon, Object>
    {
        private JLabel m_jLabel;
        private ImageIcon m_aImageIcon;
        private int w;
        private int h;

        public ResizeImage(JLabel _aLabel, ImageIcon _aImageIcon)
        {
            m_jLabel = _aLabel;
            m_aImageIcon = _aImageIcon;
            w = _aLabel.getWidth();
            h = _aLabel.getHeight();
        }

        // dont access here anything to "Event Swing Thread"
        @Override
        public ImageIcon doInBackground()
        {
            Image aImage = m_aImageIcon.getImage().getScaledInstance(w, h, Image.SCALE_AREA_AVERAGING); // SCALE_SMOOTH
            final ImageIcon aIcon = new ImageIcon(aImage);
        // m_jLabel.setIcon(aIcon);
            return aIcon;
        }

        @Override
        protected void done()
        {
            try
            {
                m_jLabel.setIcon(get());
            }
            catch (Exception e)
            {}
        }

    }

    /**
     * @param args the command line arguments
     */
    public static void main(final String args[])
    {

        // Start GUI

        java.awt.EventQueue.invokeLater(new Runnable()
        {

            public void run()
            {
                ConvwatchGUI aGUI = new ConvwatchGUI(args);
                aGUI.setTitle("Graphical Compare");
                aGUI.setPreferredSize(new Dimension(1024, 768));

                aGUI.setVisible(true);
            }
        });
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton jButton1;
    private javax.swing.JButton jButton2;
    private javax.swing.JButton jButton3;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabelCurrentPage;
    private javax.swing.JLabel jLabelDifferenceImage;
    private javax.swing.JLabel jLabelDocumentName;
    private javax.swing.JLabel jLabelOriginalImage;
    private javax.swing.JLabel jLabelReferenceImage;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanelDifference;
    private javax.swing.JPanel jPanelOriginal;
    private javax.swing.JPanel jPanelReference;
    // End of variables declaration//GEN-END:variables
}

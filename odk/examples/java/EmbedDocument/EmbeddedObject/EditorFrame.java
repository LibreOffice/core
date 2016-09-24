/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.swing.JTextArea;
import javax.swing.JFrame;
import java.io.*;
import javax.imageio.ImageIO;

public class EditorFrame extends JFrame
{
    private final OwnEmbeddedObject m_aEmbObj;
    private final JTextArea m_aTextArea;
    private BufferedImage m_aBufImage;

    private final WindowListener m_aCloser = new WindowAdapter()
    {
        @Override
        public void windowClosing( WindowEvent e )
        {
            m_aBufImage = new BufferedImage( m_aTextArea.getWidth(), m_aTextArea.getHeight(), BufferedImage.TYPE_INT_RGB );
            Graphics2D aGr = m_aBufImage.createGraphics();
            m_aTextArea.paintAll( aGr );
            aGr.dispose();

            hide();
            m_aEmbObj.CloseFrameRequest();
        }
    };

    public EditorFrame( String sName, OwnEmbeddedObject aEmbObj, int nWidth, int nHeight )
    {
        super( sName );
        m_aEmbObj = aEmbObj;
        addWindowListener( m_aCloser );
        m_aTextArea = new JTextArea( "", nWidth, nHeight );

        add( "Center", m_aTextArea );
        pack();
    }

    public String getText()
    {
        return m_aTextArea.getText();
    }

    public void setText( String aText )
    {
        m_aTextArea.setText( aText );
    }

    public Dimension getAppSize()
    {
        return m_aTextArea.getSize();
    }

    public void setAppSize( Dimension aSize )
    {
        Dimension aOwnSize = getSize();
        Dimension aAppSize = m_aTextArea.getSize();
        Dimension aToSet =
           new Dimension( (int)( aSize.getWidth() + aOwnSize.getWidth() - aAppSize.getWidth() ),
                          (int)(aSize.getHeight() + aOwnSize.getHeight() - aAppSize.getHeight() ) );

        setSize( aToSet );
        validate();
    }

    public byte[] getReplacementImage()
    {
        Dimension aDim = m_aTextArea.getSize();
        BufferedImage aBufImage = null;

        if ( m_aBufImage != null )
            aBufImage = m_aBufImage;
        else
        {
            try
            {
                int nWidth = (int)aDim.getWidth();
                int nHeight = (int)aDim.getHeight();
                aBufImage = new BufferedImage( nWidth, nHeight, BufferedImage.TYPE_INT_RGB );
                Graphics2D aGr = aBufImage.createGraphics();
                aGr.setBackground( Color.WHITE );
                aGr.clearRect( 0, 0, nWidth, nHeight );
                aGr.dispose();
            }
            catch ( java.lang.Exception e )
            {}
        }

        if ( aBufImage != null )
        {
            try
            {
                File aTmpFile = File.createTempFile( "temp", ".png" );
                ImageIO.write( aBufImage, "png", aTmpFile );

                int nLen = (int)aTmpFile.length();
                byte[] aResult = new byte[nLen];
                FileInputStream aTmpStream = new FileInputStream( aTmpFile );
                aTmpStream.read( aResult );
                aTmpStream.close();
                aTmpFile.delete();

                return aResult;
            }
            catch ( java.lang.Exception e )
            {}
        }

        return new byte[0];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

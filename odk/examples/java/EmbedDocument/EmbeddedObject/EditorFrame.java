package org.openoffice.examples.embedding;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import javax.swing.JTextArea;
import javax.swing.JFrame;
import java.io.*;
import javax.imageio.ImageIO;

import org.openoffice.examples.embedding.OwnEmbeddedObject;

public class EditorFrame extends JFrame 
{
    OwnEmbeddedObject m_aEmbObj;
    JTextArea m_aTextArea;
    BufferedImage m_aBufImage;
    
    WindowListener m_aCloser = new WindowAdapter()
    {
        public void windowClosing( WindowEvent e )
        {
            // m_aBufImage = m_aTextArea.getGraphicsConfiguration().createCompatibleImage( m_aTextArea.getWidth(), m_aTextArea.getHeight() );
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
        // setResizable( false );
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
        
        // pack();
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


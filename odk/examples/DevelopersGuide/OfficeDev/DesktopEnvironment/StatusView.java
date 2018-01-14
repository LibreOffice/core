/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// __________ Imports __________

import java.awt.*;
import javax.swing.*;

// __________ Implementation __________

/**
 * Implement a view to show status information
 * of currently loaded document of a document view.
 * It uses separate listener threads to get this information
 * and actualize it automatically, if frame broadcast changes of
 * his contained document.
 * Threads are necessary to prevent this view against deadlocks.
 * These deadlocks can occur if a listener will be notified
 * by the office in an "oneway" method and try to call back
 * to the office by using a synchronous method.
 * UNO must guarantee order of all these calls ... and if
 * the source of arrived event holds a mutex and our synchronous
 * call needs this mutex too => a deadlock occurs.
 * Why? UNO had created a new thread for our synchronous call
 * inside the office process and so exist different threads
 * for this constellation.
 *
 */
public class StatusView extends    JPanel
                        implements IShutdownListener
{


    /**
     * const
     * These URL's describe available feature states.
     */
    private static final String FEATUREURL_FONT      = "slot:10007";
    private static final String FEATUREURL_SIZE      = "slot:10015";
    private static final String FEATUREURL_BOLD      = "slot:10009";
    private static final String FEATUREURL_ITALIC    = "slot:10008";
    private static final String FEATUREURL_UNDERLINE = "slot:10014";



    /**
     * const
     * These values are used to show current state of showed feature.
     */
    private static final String FONT_OFF         = "unknown" ;
    private static final String SIZE_OFF         = "0.0"     ;
    private static final String BOLD_OFF         = "-"       ;
    private static final String ITALIC_OFF       = "-"       ;
    private static final String UNDERLINE_OFF    = "-"       ;

    private static final String FONT_ON          = ""        ;
    private static final String SIZE_ON          = ""        ;
    private static final String BOLD_ON          = "X"       ;
    private static final String ITALIC_ON        = "X"       ;
    private static final String UNDERLINE_ON     = "X"       ;



    /**
     * @member  mlaFontValue            shows status of font name
     * @member  mlaSizeValue            shows status of font size
     * @member  mlaBoldValue            shows status of font style bold
     * @member  mlaUnderlineValue       shows status of font style underline
     * @member  mlaItalicValue          shows status of font style italic
     *
     * @member  maFontListener          threadsafe(!) helper to listen for status event which describe font name
     * @member  maSizeListener          threadsafe(!) helper to listen for status event which describe font size
     * @member  maBoldListener          threadsafe(!) helper to listen for status event which describe font style bold
     * @member  maUnderlineListener     threadsafe(!) helper to listen for status event which describe font style underline
     * @member  maItalicListener        threadsafe(!) helper to listen for status event which describe font style italic
     */
    private final JLabel              m_laFontValue            ;
    private final JLabel              m_laSizeValue            ;
    private final JLabel              m_laBoldValue            ;
    private final JLabel              m_laUnderlineValue       ;
    private final JLabel              m_laItalicValue          ;

    private StatusListener      m_aFontListener          ;
    private StatusListener      m_aSizeListener          ;
    private StatusListener      m_aBoldListener          ;
    private StatusListener      m_aUnderlineListener     ;
    private StatusListener      m_aItalicListener        ;



    /**
     * ctor
     * Create view controls on startup and initialize it with default values.
     * Filling of view items can be done by special set-methods.
     * We don't start listening here! see setFrame() for that ...
     */
    StatusView()
    {
        this.setLayout(new GridBagLayout());

        GridBagConstraints aConstraint = new GridBagConstraints();
        aConstraint.anchor = GridBagConstraints.NORTHWEST;
        aConstraint.insets = new Insets(2,2,2,2);
        aConstraint.gridy  = 0;
        aConstraint.gridx  = 0;

        JLabel laFont      = new JLabel("Font"     );
        JLabel laSize      = new JLabel("Size"     );
        JLabel laBold      = new JLabel("Bold"     );
        JLabel laUnderline = new JLabel("Underline");
        JLabel laItalic    = new JLabel("Italic"   );

        m_laFontValue       = new JLabel();
        m_laSizeValue       = new JLabel();
        m_laBoldValue       = new JLabel();
        m_laUnderlineValue  = new JLabel();
        m_laItalicValue     = new JLabel();

        aConstraint.gridx = 0;
        this.add( laFont, aConstraint );
        aConstraint.gridx = 1;
        this.add( m_laFontValue, aConstraint );

        ++aConstraint.gridy;

        aConstraint.gridx = 0;
        this.add( laSize, aConstraint );
        aConstraint.gridx = 1;
        this.add( m_laSizeValue, aConstraint );

        ++aConstraint.gridy;

        aConstraint.gridx = 0;
        this.add( laSize, aConstraint );
        aConstraint.gridx = 1;
        this.add( m_laSizeValue, aConstraint );

        ++aConstraint.gridy;

        aConstraint.gridx = 0;
        this.add( laBold, aConstraint );
        aConstraint.gridx = 1;
        this.add( m_laBoldValue, aConstraint );

        ++aConstraint.gridy;

        aConstraint.gridx = 0;
        this.add( laUnderline, aConstraint );
        aConstraint.gridx = 1;
        this.add( m_laUnderlineValue, aConstraint );

        ++aConstraint.gridy;

        aConstraint.gridx = 0;
        this.add( laItalic, aConstraint );
        aConstraint.gridx = 1;
        this.add( m_laItalicValue, aConstraint );

        m_laFontValue.setEnabled     (false);
        m_laSizeValue.setEnabled     (false);
        m_laBoldValue.setEnabled     (false);
        m_laItalicValue.setEnabled   (false);
        m_laUnderlineValue.setEnabled(false);

        m_laFontValue.setText     (FONT_OFF     );
        m_laSizeValue.setText     (SIZE_OFF     );
        m_laBoldValue.setText     (BOLD_OFF     );
        m_laItalicValue.setText   (ITALIC_OFF   );
        m_laUnderlineValue.setText(UNDERLINE_OFF);
    }



    /*
     * Set new frame for this view and start listening for events immediately.
     * We create one status listener for every control we wish to update.
     * And because the environment of the frame can be changed - these
     * listener refresh himself internally for frame action events too.
     * So we register it as such frame action listener only here.
     * Rest is done automatically...
     *
     * @param xFrame
     *          will be used as source of possible status events
     */
    public void setFrame(com.sun.star.frame.XFrame xFrame)
    {
        if (xFrame==null)
            return;

        // create some listener on given frame for available status events
        // Created listener instances will register themselves on this frame and
        // show it received information automatically on set UI controls.
        m_aFontListener      = new StatusListener(m_laFontValue     ,FONT_ON     ,FONT_OFF     ,xFrame, FEATUREURL_FONT     );
        m_aSizeListener      = new StatusListener(m_laSizeValue     ,SIZE_ON     ,SIZE_OFF     ,xFrame, FEATUREURL_SIZE     );
        m_aBoldListener      = new StatusListener(m_laBoldValue     ,BOLD_ON     ,BOLD_OFF     ,xFrame, FEATUREURL_BOLD     );
        m_aItalicListener    = new StatusListener(m_laItalicValue   ,ITALIC_ON   ,ITALIC_OFF   ,xFrame, FEATUREURL_ITALIC   );
        m_aUnderlineListener = new StatusListener(m_laUnderlineValue,UNDERLINE_ON,UNDERLINE_OFF,xFrame, FEATUREURL_UNDERLINE);

        m_aFontListener.startListening();
        m_aSizeListener.startListening();
        m_aBoldListener.startListening();
        m_aItalicListener.startListening();
        m_aUnderlineListener.startListening();
    }



    /**
     * If this java application shutdown - we must cancel all current existing
     * listener connections. Otherwise the office will run into some
     * DisposedExceptions if it tries to use these forgotten listener references.
     * And of course it can die doing that.
     * We are registered at a central object to be informed if the VM will exit.
     * So we can react.
     */
    public void shutdown()
    {
        m_aFontListener.shutdown();
        m_aSizeListener.shutdown();
        m_aBoldListener.shutdown();
        m_aItalicListener.shutdown();
        m_aUnderlineListener.shutdown();

        m_aFontListener      = null;
        m_aSizeListener      = null;
        m_aBoldListener      = null;
        m_aItalicListener    = null;
        m_aUnderlineListener = null;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

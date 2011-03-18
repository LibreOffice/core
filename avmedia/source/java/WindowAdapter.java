/*************************************************************************
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
 ************************************************************************/

import sun.awt.*;
import com.sun.star.awt.*;
import com.sun.star.lang.*;
import java.util.*;
import javax.swing.*;

public class WindowAdapter
{
    private java.awt.Frame  maFrame;
    private LinkedList      maEventListeners = new LinkedList();
    private LinkedList      maWindowListeners = new LinkedList();
    private LinkedList      maFocusListeners = new LinkedList();
    private LinkedList      maKeyListeners = new LinkedList();
    private LinkedList      maMouseListeners = new LinkedList();
    private LinkedList      maMouseMotionListeners = new LinkedList();
    private LinkedList      maPaintListeners = new LinkedList();
    private boolean         mbShift = false, mbMod1 = false, mbMod2 = false;

    // -----------------
    // - WindowAdapter -
    // -----------------

    public WindowAdapter( int windowHandle )
    {
        maFrame = SystemWindowAdapter.createFrame( windowHandle );
    }

    //----------------------------------------------------------------------------------

    public java.awt.Frame getJavaFrame()
    {
        return maFrame;
    }

    //----------------------------------------------------------------------------------

    private short implGetUNOKeyCode( int nJavaKeyCode )
    {
        short nRet = 0;

        switch( nJavaKeyCode )
        {
            case( java.awt.event.KeyEvent.VK_NUMPAD0 ):
            case( java.awt.event.KeyEvent.VK_0 ):       nRet = com.sun.star.awt.Key.NUM0; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD1 ):
            case( java.awt.event.KeyEvent.VK_1 ):       nRet = com.sun.star.awt.Key.NUM1; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD2 ):
            case( java.awt.event.KeyEvent.VK_2 ):       nRet = com.sun.star.awt.Key.NUM2; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD3 ):
            case( java.awt.event.KeyEvent.VK_3 ):       nRet = com.sun.star.awt.Key.NUM3; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD4 ):
            case( java.awt.event.KeyEvent.VK_4 ):       nRet = com.sun.star.awt.Key.NUM4; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD5 ):
            case( java.awt.event.KeyEvent.VK_5 ):       nRet = com.sun.star.awt.Key.NUM5; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD6 ):
            case( java.awt.event.KeyEvent.VK_6 ):       nRet = com.sun.star.awt.Key.NUM6; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD7 ):
            case( java.awt.event.KeyEvent.VK_7 ):       nRet = com.sun.star.awt.Key.NUM7; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD8 ):
            case( java.awt.event.KeyEvent.VK_8 ):       nRet = com.sun.star.awt.Key.NUM8; break;
            case( java.awt.event.KeyEvent.VK_NUMPAD9 ):
            case( java.awt.event.KeyEvent.VK_9 ):       nRet = com.sun.star.awt.Key.NUM9; break;

            case( java.awt.event.KeyEvent.VK_A ): nRet = com.sun.star.awt.Key.A; break;
            case( java.awt.event.KeyEvent.VK_B ): nRet = com.sun.star.awt.Key.B; break;
            case( java.awt.event.KeyEvent.VK_C ): nRet = com.sun.star.awt.Key.C; break;
            case( java.awt.event.KeyEvent.VK_D ): nRet = com.sun.star.awt.Key.D; break;
            case( java.awt.event.KeyEvent.VK_E ): nRet = com.sun.star.awt.Key.E; break;
            case( java.awt.event.KeyEvent.VK_F ): nRet = com.sun.star.awt.Key.F; break;
            case( java.awt.event.KeyEvent.VK_G ): nRet = com.sun.star.awt.Key.G; break;
            case( java.awt.event.KeyEvent.VK_H ): nRet = com.sun.star.awt.Key.H; break;
            case( java.awt.event.KeyEvent.VK_I ): nRet = com.sun.star.awt.Key.I; break;
            case( java.awt.event.KeyEvent.VK_J ): nRet = com.sun.star.awt.Key.J; break;
            case( java.awt.event.KeyEvent.VK_K ): nRet = com.sun.star.awt.Key.K; break;
            case( java.awt.event.KeyEvent.VK_L ): nRet = com.sun.star.awt.Key.L; break;
            case( java.awt.event.KeyEvent.VK_M ): nRet = com.sun.star.awt.Key.M; break;
            case( java.awt.event.KeyEvent.VK_N ): nRet = com.sun.star.awt.Key.N; break;
            case( java.awt.event.KeyEvent.VK_O ): nRet = com.sun.star.awt.Key.O; break;
            case( java.awt.event.KeyEvent.VK_P ): nRet = com.sun.star.awt.Key.P; break;
            case( java.awt.event.KeyEvent.VK_Q ): nRet = com.sun.star.awt.Key.Q; break;
            case( java.awt.event.KeyEvent.VK_R ): nRet = com.sun.star.awt.Key.R; break;
            case( java.awt.event.KeyEvent.VK_S ): nRet = com.sun.star.awt.Key.S; break;
            case( java.awt.event.KeyEvent.VK_T ): nRet = com.sun.star.awt.Key.T; break;
            case( java.awt.event.KeyEvent.VK_U ): nRet = com.sun.star.awt.Key.U; break;
            case( java.awt.event.KeyEvent.VK_V ): nRet = com.sun.star.awt.Key.V; break;
            case( java.awt.event.KeyEvent.VK_W ): nRet = com.sun.star.awt.Key.W; break;
            case( java.awt.event.KeyEvent.VK_X ): nRet = com.sun.star.awt.Key.X; break;
            case( java.awt.event.KeyEvent.VK_Y ): nRet = com.sun.star.awt.Key.Y; break;
            case( java.awt.event.KeyEvent.VK_Z ): nRet = com.sun.star.awt.Key.Z; break;

            case( java.awt.event.KeyEvent.VK_F1 ): nRet = com.sun.star.awt.Key.F1; break;
            case( java.awt.event.KeyEvent.VK_F2 ): nRet = com.sun.star.awt.Key.F2; break;
            case( java.awt.event.KeyEvent.VK_F3 ): nRet = com.sun.star.awt.Key.F3; break;
            case( java.awt.event.KeyEvent.VK_F4 ): nRet = com.sun.star.awt.Key.F4; break;
            case( java.awt.event.KeyEvent.VK_F5 ): nRet = com.sun.star.awt.Key.F5; break;
            case( java.awt.event.KeyEvent.VK_F6 ): nRet = com.sun.star.awt.Key.F6; break;
            case( java.awt.event.KeyEvent.VK_F7 ): nRet = com.sun.star.awt.Key.F7; break;
            case( java.awt.event.KeyEvent.VK_F8 ): nRet = com.sun.star.awt.Key.F8; break;
            case( java.awt.event.KeyEvent.VK_F9 ): nRet = com.sun.star.awt.Key.F9; break;
            case( java.awt.event.KeyEvent.VK_F10 ): nRet = com.sun.star.awt.Key.F10; break;
            case( java.awt.event.KeyEvent.VK_F11 ): nRet = com.sun.star.awt.Key.F11; break;
            case( java.awt.event.KeyEvent.VK_F12 ): nRet = com.sun.star.awt.Key.F12; break;
            case( java.awt.event.KeyEvent.VK_F13 ): nRet = com.sun.star.awt.Key.F13; break;
            case( java.awt.event.KeyEvent.VK_F14 ): nRet = com.sun.star.awt.Key.F14; break;
            case( java.awt.event.KeyEvent.VK_F15 ): nRet = com.sun.star.awt.Key.F15; break;
            case( java.awt.event.KeyEvent.VK_F16 ): nRet = com.sun.star.awt.Key.F16; break;
            case( java.awt.event.KeyEvent.VK_F17 ): nRet = com.sun.star.awt.Key.F17; break;
            case( java.awt.event.KeyEvent.VK_F18 ): nRet = com.sun.star.awt.Key.F18; break;
            case( java.awt.event.KeyEvent.VK_F19 ): nRet = com.sun.star.awt.Key.F19; break;
            case( java.awt.event.KeyEvent.VK_F20 ): nRet = com.sun.star.awt.Key.F20; break;
            case( java.awt.event.KeyEvent.VK_F21 ): nRet = com.sun.star.awt.Key.F21; break;
            case( java.awt.event.KeyEvent.VK_F22 ): nRet = com.sun.star.awt.Key.F22; break;
            case( java.awt.event.KeyEvent.VK_F23 ): nRet = com.sun.star.awt.Key.F23; break;
            case( java.awt.event.KeyEvent.VK_F24 ): nRet = com.sun.star.awt.Key.F24; break;

            case( java.awt.event.KeyEvent.VK_UP ):          nRet = com.sun.star.awt.Key.UP; break;
            case( java.awt.event.KeyEvent.VK_DOWN):         nRet = com.sun.star.awt.Key.DOWN; break;
            case( java.awt.event.KeyEvent.VK_LEFT ):        nRet = com.sun.star.awt.Key.LEFT; break;
            case( java.awt.event.KeyEvent.VK_RIGHT ):       nRet = com.sun.star.awt.Key.RIGHT; break;

            case( java.awt.event.KeyEvent.VK_HOME ):        nRet = com.sun.star.awt.Key.HOME; break;
            case( java.awt.event.KeyEvent.VK_END ):         nRet = com.sun.star.awt.Key.END; break;

            case( java.awt.event.KeyEvent.VK_PAGE_UP ):     nRet = com.sun.star.awt.Key.PAGEUP; break;
            case( java.awt.event.KeyEvent.VK_PAGE_DOWN ):   nRet = com.sun.star.awt.Key.PAGEDOWN; break;

            case( java.awt.event.KeyEvent.VK_ENTER ):       nRet = com.sun.star.awt.Key.RETURN; break;
            case( java.awt.event.KeyEvent.VK_ESCAPE ):      nRet = com.sun.star.awt.Key.ESCAPE; break;

            case( java.awt.event.KeyEvent.VK_TAB ):         nRet = com.sun.star.awt.Key.TAB; break;
            case( java.awt.event.KeyEvent.VK_BACK_SPACE ):  nRet = com.sun.star.awt.Key.BACKSPACE; break;
            case( java.awt.event.KeyEvent.VK_SPACE ):       nRet = com.sun.star.awt.Key.SPACE; break;
            case( java.awt.event.KeyEvent.VK_INSERT):       nRet = com.sun.star.awt.Key.INSERT; break;
            case( java.awt.event.KeyEvent.VK_DELETE):       nRet = com.sun.star.awt.Key.DELETE; break;
            case( java.awt.event.KeyEvent.VK_ADD ):         nRet = com.sun.star.awt.Key.ADD; break;
            case( java.awt.event.KeyEvent.VK_SUBTRACT ):    nRet = com.sun.star.awt.Key.SUBTRACT; break;
            case( java.awt.event.KeyEvent.VK_MULTIPLY ):    nRet = com.sun.star.awt.Key.MULTIPLY; break;
            case( java.awt.event.KeyEvent.VK_DIVIDE ):      nRet = com.sun.star.awt.Key.DIVIDE; break;
            case( java.awt.event.KeyEvent.VK_DECIMAL ):     nRet = com.sun.star.awt.Key.POINT; break;
            // case( java.awt.event.KeyEvent.VK_ COMMA; break;
            case( java.awt.event.KeyEvent.VK_LESS ):        nRet = com.sun.star.awt.Key.LESS; break;
            case( java.awt.event.KeyEvent.VK_GREATER ):     nRet = com.sun.star.awt.Key.GREATER; break;
            case( java.awt.event.KeyEvent.VK_EQUALS ):      nRet = com.sun.star.awt.Key.EQUAL; break;
            // case( java.awt.event.KeyEvent.VK_ OPEN; break;
            // case( java.awt.event.KeyEvent.VK_ CUT; break;
            // case( java.awt.event.KeyEvent.VK_ COPY; break;
            // case( java.awt.event.KeyEvent.VK_ PASTE; break;
            // case( java.awt.event.KeyEvent.VK_ UNDO; break;
            // case( java.awt.event.KeyEvent.VK_ REPEAT; break;
            // case( java.awt.event.KeyEvent.VK_ FIND; break;
            // case( java.awt.event.KeyEvent.VK_ PROPERTIES; break;
            // case( java.awt.event.KeyEvent.VK_ FRONT; break;
            // case( java.awt.event.KeyEvent.VK_ CONTEXTMENU; break;
            // case( java.awt.event.KeyEvent.VK_ HELP; break;

            default:
            break;
         }

         return nRet;
    }

    //----------------------------------------------------------------------------------

    public void fireKeyEvent( java.awt.event.KeyEvent aEvt )
    {
        if( maKeyListeners.size() > 0 )
        {
            boolean bProcess = false, bPressed = false;

            if( java.awt.event.KeyEvent.KEY_PRESSED == aEvt.getID() )
            {
                switch( aEvt.getKeyCode() )
                {
                    case( java.awt.event.KeyEvent.VK_SHIFT ): mbShift = true; break;
                    case( java.awt.event.KeyEvent.VK_CONTROL ): mbMod1 = true; break;
                    case( java.awt.event.KeyEvent.VK_ALT ): mbMod2 = true; break;

                    default:
                    {
                        bProcess = bPressed = true;
                    }
                    break;
                }
            }
            else if( java.awt.event.KeyEvent.KEY_RELEASED == aEvt.getID() )
            {
                switch( aEvt.getKeyCode() )
                {
                    case( java.awt.event.KeyEvent.VK_SHIFT ): mbShift = false; break;
                    case( java.awt.event.KeyEvent.VK_CONTROL ): mbMod1 = false; break;
                    case( java.awt.event.KeyEvent.VK_ALT ): mbMod2 = false; break;

                    default:
                    {
                        bProcess = true;
                    }
                    break;
                }
            }

            if( bProcess )
            {
                KeyEvent aUNOEvt = new KeyEvent();

                aUNOEvt.Modifiers = 0;

                if( mbShift )
                    aUNOEvt.Modifiers |= com.sun.star.awt.KeyModifier.SHIFT;

                if( mbMod1 )
                    aUNOEvt.Modifiers |= com.sun.star.awt.KeyModifier.MOD1;

                if( mbMod2 )
                    aUNOEvt.Modifiers |= com.sun.star.awt.KeyModifier.MOD2;

                aUNOEvt.KeyCode = implGetUNOKeyCode( aEvt.getKeyCode() );
                aUNOEvt.KeyChar = aEvt.getKeyChar();
                aUNOEvt.KeyFunc = com.sun.star.awt.KeyFunction.DONTKNOW;

                ListIterator aIter = maKeyListeners.listIterator( 0 );

                while( aIter.hasNext() )
                {
                    if( bPressed )
                        ( (XKeyListener) aIter.next() ).keyPressed( aUNOEvt );
                    else
                        ( (XKeyListener) aIter.next() ).keyReleased( aUNOEvt );
                }
            }
        }
    }

    //----------------------------------------------------------------------------------

    public void fireMouseEvent( java.awt.event.MouseEvent aEvt )
    {
        MouseEvent aUNOEvt = new MouseEvent();

        aUNOEvt.Modifiers = 0;
        aUNOEvt.Buttons = 0;
        aUNOEvt.X = aEvt.getX();
        aUNOEvt.Y = aEvt.getY();
         aUNOEvt.PopupTrigger = false;

        // Modifiers
        if( aEvt.isShiftDown() )
            aUNOEvt.Modifiers |= com.sun.star.awt.KeyModifier.SHIFT;

        if( aEvt.isControlDown() )
            aUNOEvt.Modifiers |= com.sun.star.awt.KeyModifier.MOD1;

        if( aEvt.isAltDown() )
            aUNOEvt.Modifiers |= com.sun.star.awt.KeyModifier.MOD2;

        // Buttons
        if( SwingUtilities.isLeftMouseButton( aEvt ) )
            aUNOEvt.Buttons |= com.sun.star.awt.MouseButton.LEFT;

        if( SwingUtilities.isMiddleMouseButton( aEvt ) )
            aUNOEvt.Buttons |= com.sun.star.awt.MouseButton.MIDDLE;

        if( SwingUtilities.isRightMouseButton( aEvt ) )
            aUNOEvt.Buttons |= com.sun.star.awt.MouseButton.RIGHT;

        // event type
        if( java.awt.event.MouseEvent.MOUSE_PRESSED == aEvt.getID() )
        {
            ListIterator aIter = maMouseListeners.listIterator( 0 );

            aUNOEvt.ClickCount = 1;

            while( aIter.hasNext() )
                ( (XMouseListener) aIter.next() ).mousePressed( aUNOEvt );
        }
        else if( java.awt.event.MouseEvent.MOUSE_RELEASED == aEvt.getID() )
        {
            ListIterator aIter = maMouseListeners.listIterator( 0 );

            aUNOEvt.ClickCount = 1;

            while( aIter.hasNext() )
                ( (XMouseListener) aIter.next() ).mouseReleased( aUNOEvt );
        }
        else if( java.awt.event.MouseEvent.MOUSE_DRAGGED == aEvt.getID() )
        {
            ListIterator aIter = maMouseMotionListeners.listIterator( 0 );

            aUNOEvt.ClickCount = 0;

            while( aIter.hasNext() )
                ( (XMouseMotionListener) aIter.next() ).mouseDragged( aUNOEvt );
        }
        else if( java.awt.event.MouseEvent.MOUSE_MOVED == aEvt.getID() )
        {
            ListIterator aIter = maMouseMotionListeners.listIterator( 0 );

            aUNOEvt.ClickCount = 0;

            while( aIter.hasNext() )
                ( (XMouseMotionListener) aIter.next() ).mouseMoved( aUNOEvt );
        }
    }

    //----------------------------------------------------------------------------------

    public void fireFocusEvent( java.awt.event.FocusEvent aEvt )
    {
        if( java.awt.event.FocusEvent.FOCUS_GAINED == aEvt.getID() )
        {
            ListIterator aIter = maFocusListeners.listIterator( 0 );
            FocusEvent   aUNOEvt = new FocusEvent();

            while( aIter.hasNext() )
            {
                ( (XFocusListener) aIter.next() ).focusGained( aUNOEvt );
            }
        }
    }

    //----------------------------------------------------------------------------------

    public void fireDisposingEvent()
    {
        ListIterator aIter = maEventListeners.listIterator( 0 );

        while( aIter.hasNext() )
        {
            ( (XEventListener) aIter.next() ).disposing( new com.sun.star.lang.EventObject() );
        }
    }

    // --------------------
    // - XWindow methods  -
    // --------------------

    public void setPosSize( int X, int Y, int Width, int Height, short Flags )
    {
        maFrame.setBounds( X, Y, Width, Height );
    }

    //----------------------------------------------------------------------------------

    public com.sun.star.awt.Rectangle getPosSize(  )
    {
        java.awt.Rectangle bounds = maFrame.getBounds();
        return new com.sun.star.awt.Rectangle( bounds.x, bounds.y, bounds.width, bounds.height );
    }

    //----------------------------------------------------------------------------------

    public void setVisible( boolean visible )
    {
         maFrame.setVisible( visible );
    }

    //----------------------------------------------------------------------------------

    public void setEnable( boolean enable )
    {
        maFrame.setEnabled( enable );
    }

    //----------------------------------------------------------------------------------

    public void setFocus()
    {
    }

    //----------------------------------------------------------------------------------

    public void addEventListener( XEventListener xListener )
    {
        if( xListener != null )
            maEventListeners.add( xListener );
    }

    //----------------------------------------------------------------------------------

    public void removeEventListener( XEventListener xListener )
    {
        if( xListener != null )
            maEventListeners.remove( xListener );
    }

    //----------------------------------------------------------------------------------

    public void addWindowListener( XWindowListener xListener )
    {
        if( xListener != null )
            maWindowListeners.add( xListener );
    }

    //----------------------------------------------------------------------------------

    public void removeWindowListener( XWindowListener xListener )
    {
        if( xListener != null )
            maWindowListeners.remove( xListener );
    }

    //----------------------------------------------------------------------------------

    public void addFocusListener( XFocusListener xListener )
    {
        if( xListener != null )
            maFocusListeners.add( xListener );
    }

    //----------------------------------------------------------------------------------

    public void removeFocusListener( XFocusListener xListener )
    {
        if( xListener != null )
            maFocusListeners.remove( xListener );
    }

    //----------------------------------------------------------------------------------

    public void addKeyListener( XKeyListener xListener )
    {
        if( xListener != null )
            maKeyListeners.add( xListener );
    }

    //----------------------------------------------------------------------------------

    public void removeKeyListener( XKeyListener xListener )
    {
        if( xListener != null )
            maKeyListeners.remove( xListener );
    }

    //----------------------------------------------------------------------------------

    public void addMouseListener( XMouseListener xListener )
    {
        if( xListener != null )
            maMouseListeners.add( xListener );
    }

    //----------------------------------------------------------------------------------

    public void removeMouseListener( XMouseListener xListener )
    {
        if( xListener != null )
            maMouseListeners.remove( xListener );
    }

    //----------------------------------------------------------------------------------

    public void addMouseMotionListener( XMouseMotionListener xListener )
    {
        if( xListener != null )
            maMouseMotionListeners.add( xListener );
    }

    //----------------------------------------------------------------------------------

    public void removeMouseMotionListener( XMouseMotionListener xListener )
    {
        if( xListener != null )
            maMouseMotionListeners.remove( xListener );
    }

    //----------------------------------------------------------------------------------

    public void addPaintListener( XPaintListener xListener )
    {
        if( xListener != null )
            maPaintListeners.add( xListener );
    }

    //----------------------------------------------------------------------------------

    public void removePaintListener( XPaintListener xListener )
    {
        if( xListener != null )
            maPaintListeners.remove( xListener );
    }
}

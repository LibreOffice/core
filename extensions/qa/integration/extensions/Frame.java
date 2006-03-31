/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Frame.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-31 12:17:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package integration.extensions;

import com.sun.star.uno.*;
import com.sun.star.frame.*;
import com.sun.star.task.XStatusIndicatorFactory;
import com.sun.star.util.XCloseable;

/** wraps the coms.sun.star.frame.Frame service
 *
 * @author fs93730
 */
public class Frame implements   XFrame,
                                XDispatchProvider,
                                XDispatchProviderInterception,
                                XFramesSupplier,
                                XStatusIndicatorFactory,
                                XCloseable
{
    private XFrame                          m_frame;
    private XDispatchProvider               m_dispatchProvider;
    private XDispatchProviderInterception   m_dispatchProviderInterception;
    private XFramesSupplier                 m_framesSupplier;
    private XStatusIndicatorFactory         m_statusIndicatorFactory;
    private XCloseable                      m_closeable;

    /** Creates a new instance of Frame */
    public Frame( Object _frameComponent )
    {
        if ( _frameComponent != null )
        {
            m_frame = (XFrame)UnoRuntime.queryInterface( XFrame.class, _frameComponent );
            m_dispatchProvider = (XDispatchProvider)UnoRuntime.queryInterface( XDispatchProvider.class, _frameComponent );
            m_dispatchProviderInterception = (XDispatchProviderInterception)UnoRuntime.queryInterface( XDispatchProviderInterception.class, _frameComponent );
            m_framesSupplier = (XFramesSupplier)UnoRuntime.queryInterface( XFramesSupplier.class, _frameComponent );
            m_statusIndicatorFactory = (XStatusIndicatorFactory)UnoRuntime.queryInterface( XStatusIndicatorFactory.class, _frameComponent );
            m_closeable = (XCloseable)UnoRuntime.queryInterface( XCloseable.class, _frameComponent );
        }
    }

    public XFrame getXFrame()
    {
        return m_frame;
    }

    public void activate()
    {
        m_frame.activate();
    }

    public void addEventListener(com.sun.star.lang.XEventListener _eventListener)
    {
        m_frame.addEventListener( _eventListener );
    }

    public void addFrameActionListener(XFrameActionListener _frameActionListener)
    {
        m_frame.addFrameActionListener( _frameActionListener );
    }

    public void contextChanged()
    {
        m_frame.contextChanged();
    }

    public com.sun.star.task.XStatusIndicator createStatusIndicator()
    {
        return m_statusIndicatorFactory.createStatusIndicator();
    }

    public void deactivate()
    {
        m_frame.deactivate();
    }

    public void dispose()
    {
        m_frame.dispose();
    }

    public XFrame findFrame(String str, int param)
    {
        return m_frame.findFrame( str, param );
    }

    public XFrame getActiveFrame()
    {
        return m_framesSupplier.getActiveFrame();
    }

    public com.sun.star.awt.XWindow getComponentWindow()
    {
        return m_frame.getComponentWindow();
    }

    public com.sun.star.awt.XWindow getContainerWindow()
    {
        return m_frame.getContainerWindow();
    }

    public XController getController()
    {
        return m_frame.getController();
    }

    public XFramesSupplier getCreator()
    {
        return m_frame.getCreator();
    }

    public XFrames getFrames()
    {
        return m_framesSupplier.getFrames();
    }

    public String getName()
    {
        return m_frame.getName();
    }

    public void initialize(com.sun.star.awt.XWindow _window)
    {
        m_frame.initialize( _window );
    }

    public boolean isActive()
    {
        return m_frame.isActive();
    }

    public boolean isTop()
    {
        return m_frame.isTop();
    }

    public XDispatch queryDispatch(com.sun.star.util.URL _url, String _str, int _param)
    {
        return m_dispatchProvider.queryDispatch( _url, _str, _param );
    }

    public XDispatch[] queryDispatches(DispatchDescriptor[] dispatchDescriptor)
    {
        return m_dispatchProvider.queryDispatches( dispatchDescriptor );
    }

    public void registerDispatchProviderInterceptor(XDispatchProviderInterceptor _dispatchProviderInterceptor)
    {
        m_dispatchProviderInterception.registerDispatchProviderInterceptor( _dispatchProviderInterceptor );
    }

    public void releaseDispatchProviderInterceptor(XDispatchProviderInterceptor _dispatchProviderInterceptor)
    {
        m_dispatchProviderInterception.releaseDispatchProviderInterceptor( _dispatchProviderInterceptor );
    }

    public void removeEventListener(com.sun.star.lang.XEventListener _eventListener)
    {
        m_frame.removeEventListener( _eventListener );
    }

    public void removeFrameActionListener(XFrameActionListener _frameActionListener)
    {
        m_frame.removeFrameActionListener( _frameActionListener );
    }

    public void setActiveFrame(XFrame _frame)
    {
        m_framesSupplier.setActiveFrame( _frame );
    }

    public boolean setComponent(com.sun.star.awt.XWindow _window, XController _controller)
    {
        return m_frame.setComponent( _window, _controller );
    }

    public void setCreator(XFramesSupplier _framesSupplier)
    {
        m_frame.setCreator( _framesSupplier );
    }

    public void setName(String str)
    {
        m_frame.setName( str );
    }

    public void close(boolean _deliverOwnership) throws com.sun.star.util.CloseVetoException
    {
        m_closeable.close( _deliverOwnership );
    }

    public void removeCloseListener(com.sun.star.util.XCloseListener _closeListener)
    {
        m_closeable.removeCloseListener( _closeListener );
    }

    public void addCloseListener(com.sun.star.util.XCloseListener _closeListener)
    {
        m_closeable.addCloseListener( _closeListener );
    }
}

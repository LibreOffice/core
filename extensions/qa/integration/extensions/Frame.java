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

package integration.extensions;

import com.sun.star.uno.*;
import com.sun.star.frame.*;
import com.sun.star.task.XStatusIndicatorFactory;
import com.sun.star.util.XCloseable;

/**
 * wraps the coms.sun.star.frame.Frame service
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

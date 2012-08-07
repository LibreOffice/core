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

import com.sun.star.uno.UnoRuntime;

import java.lang.*;
import javax.swing.*;
import java.util.Vector;

// __________ Implementation __________

/**
 * This class can be used to intercept dispatched URL's
 * on any frame used in this demo application.
 * It intercept all URL's wich try to create a new empty frame.
 * (e.g. "private:factory/swriter")
 * Nobody can guarantee that this interception will be realy used -
 * because another interceptor (registered at a later time then this one!)
 * will be called before this one.
 * Implementation is executed inside a new thread to prevent application
 * against possible deadlocks. This deadlocks can occure if
 * synchronous/asynchronous ... normal ones and oneway calls are mixed.
 * Notifications of listener will be oneway mostly - her reactions can
 * be synchronous then. => deadlocks are possible
 *
 * @author     Andreas Schl&uuml;ns
 */
public class Interceptor implements com.sun.star.frame.XFrameActionListener,
                                    com.sun.star.frame.XDispatchProviderInterceptor,
                                    com.sun.star.frame.XDispatchProvider,
                                    com.sun.star.frame.XDispatch,
                                    com.sun.star.frame.XInterceptorInfo,
                                    IShutdownListener,
                                    IOnewayLink
{
    // ____________________

    /**
     * const
     * All these URL's are intercepted by this implementation.
     */
    private static final String[] INTERCEPTED_URLS = { "private:factory/*" ,
                                                       ".uno:SaveAs"       ,
                                                       "slot:5300"         ,
                                                       ".uno:Quit"         };

    // ____________________

    /**
     * @member m_xMaster     use this interceptor if he doesn't handle queried dispatch request
     * @member m_xSlave      we can forward all unhandled requests to this slave interceptor
     * @member m_xFrame      intercepted frame
     * @member m_bDead       there exist more then one way to finish an object of this class - we must know it sometimes
     */
    private com.sun.star.frame.XDispatchProvider m_xMaster            ;
    private com.sun.star.frame.XDispatchProvider m_xSlave             ;
    private com.sun.star.frame.XFrame            m_xFrame             ;
    private boolean                              m_bIsActionListener  ;
    private boolean                              m_bIsRegistered      ;
    private boolean                              m_bDead              ;

    // ____________________

    /**
     * ctor
     * Initialize the new interceptor. Given frame reference can be used to
     * register this interceptor on it automaticly later.
     *
     * @seealso startListening()
     *
     * @param xFrame
     *          this interceptor will register himself at this frame to intercept dispatched URLs
     */
    Interceptor(/*IN*/ com.sun.star.frame.XFrame xFrame)
    {
        m_xFrame            = xFrame ;
        m_xSlave            = null   ;
        m_xMaster           = null   ;
        m_bIsRegistered     = false  ;
        m_bIsActionListener = false  ;
        m_bDead             = false  ;
    }

    //_____________________

    /**
     * start working as frame action listener realy.
     * We will be frame action listener here. In case
     * we get a frame action which indicates, that we should
     * update our interception. Because such using of an interecptor
     * isn't guaranteed - in case a newer one was registered ...
     */
    public void startListening()
    {
        com.sun.star.frame.XFrame xFrame = null;
        synchronized(this)
        {
            if (m_bDead)
                return;
            if (m_xFrame==null)
                return;
            if (m_bIsActionListener==true)
                return;
            xFrame = m_xFrame;
        }
        m_xFrame.addFrameActionListener(this);
        synchronized(this)
        {
            m_bIsActionListener=true;
        }
    }

    //_____________________

    /**
     * In case we got an oneway listener callback - we had to use the office
     * asynchronous then. This method is the callback from the started thread
     * (started inside the original oneway method). We found all parameters of
     * the original request packed inside a vector. Here we unpack it and
     * call the right internal helper method, which implements the right
     * funtionality.
     *
     * @seealso frameAction()
     * @seealso dispatch()
     *
     * @param nRequest
     *          indicates, which was the original request (identifies the
     *          original called method)
     *
     * @param lParams
     *          the vector with all packed parameters of the original request
     */
    public void execOneway(/*IN*/ int nRequest,/*IN*/ Vector lParams )
    {
        synchronized(this)
        {
            if (m_bDead)
                return;
        }

        // was it frameAction()?
        if (nRequest==OnewayExecutor.REQUEST_FRAMEACTION)
        {
            com.sun.star.frame.FrameActionEvent[] lOutAction   = new com.sun.star.frame.FrameActionEvent[1];
            Vector[]                              lInParams    = new Vector[1];
                                                  lInParams[0] = lParams;

            OnewayExecutor.codeFrameAction( OnewayExecutor.DECODE_PARAMS ,
                                            lInParams                    ,
                                            lOutAction                   );
            impl_frameAction(lOutAction[0]);
        }
        else
        // was it dispatch()?
        if (nRequest==OnewayExecutor.REQUEST_DISPATCH)
        {
            com.sun.star.util.URL[]              lOutURL      = new com.sun.star.util.URL[1];
            com.sun.star.beans.PropertyValue[][] lOutProps    = new com.sun.star.beans.PropertyValue[1][];
            Vector[]                             lInParams    = new Vector[1];
                                                 lInParams[0] = lParams;

            OnewayExecutor.codeDispatch( OnewayExecutor.DECODE_PARAMS ,
                                         lInParams                    ,
                                         lOutURL                      ,
                                         lOutProps                    );
            impl_dispatch(lOutURL[0],lOutProps[0]);
        }
    }

    // ____________________

    /**
     * call back for frame action events
     * We use it to update our interception. Because if a new component was loaded into
     * the frame or another interceptor was registered, we should refresh our connection
     * to the frame. Otherwhise we can't guarantee full functionality here.
     *
     * Note: Don't react synchronous in an asynchronous listener callback. So use a thread
     * here to update anything.
     *
     * @seealso impl_frameAction()
     *
     * @param aEvent
     *          describes the action
     */
    public /*ONEWAY*/  void frameAction(/*IN*/ com.sun.star.frame.FrameActionEvent aEvent)
    {
        synchronized(this)
        {
            if (m_bDead)
                return;
        }

        boolean bHandle = false;
        switch(aEvent.Action.getValue())
        {
            case com.sun.star.frame.FrameAction.COMPONENT_ATTACHED_value   : bHandle=true; break;
            case com.sun.star.frame.FrameAction.COMPONENT_DETACHING_value  : bHandle=true; break;
            case com.sun.star.frame.FrameAction.COMPONENT_REATTACHED_value : bHandle=true; break;
            // Don't react for CONTEXT_CHANGED here. Ok it indicates, that may another interceptor
            // was registered at the frame ... but if we register ourself there - we get a context
            // changed too :-( Best way to produce a never ending recursion ...
            // May be that somewhere find a safe mechanism to detect own produced frame action events
            // and ignore it.
            case com.sun.star.frame.FrameAction.CONTEXT_CHANGED_value :
                    System.out.println("Time to update interception ... but may it will start a recursion. So I let it :-(");
                    bHandle=false;
                    break;
        }

        // ignore some events
        if (! bHandle)
            return;

        // pack the event and start thread - which call us back later
        Vector[]                              lOutParams   = new Vector[1];
        com.sun.star.frame.FrameActionEvent[] lInAction    = new com.sun.star.frame.FrameActionEvent[1];
                                              lInAction[0] = aEvent;

        OnewayExecutor.codeFrameAction( OnewayExecutor.ENCODE_PARAMS ,
                                        lOutParams                   ,
                                        lInAction                    );
        OnewayExecutor aExecutor = new OnewayExecutor( (IOnewayLink)this                  ,
                                                       OnewayExecutor.REQUEST_FRAMEACTION ,
                                                       lOutParams[0]                      );
        aExecutor.start();
    }

    // ____________________

    /**
     * Indicates using of us as an interceptor.
     * Now we have to react for the requests, we are registered.
     * That means: load new empty documents - triggered by the new menu of the office.
     * Because it's oneway - use thread for loading!
     *
     * @seealso impl_dispatch()
     *
     * @param aURL
     *          describes the document, which should be loaded
     *
     * @param lArguments
     *          optional parameters for loading
     */
    public /*ONEWAY*/ void dispatch(/*IN*/ com.sun.star.util.URL aURL,/*IN*/ com.sun.star.beans.PropertyValue[] lArguments)
    {
        synchronized(this)
        {
            if (m_bDead)
                return;
        }

        Vector[]                             lOutParams      = new Vector[1];
        com.sun.star.util.URL[]              lInURL          = new com.sun.star.util.URL[1];
        com.sun.star.beans.PropertyValue[][] lInArguments    = new com.sun.star.beans.PropertyValue[1][];
                                             lInURL[0]       = aURL      ;
                                             lInArguments[0] = lArguments;

        OnewayExecutor.codeDispatch( OnewayExecutor.ENCODE_PARAMS ,
                                     lOutParams                   ,
                                     lInURL                       ,
                                     lInArguments                 );
        OnewayExecutor aExecutor = new OnewayExecutor( (IOnewayLink)this               ,
                                                       OnewayExecutor.REQUEST_DISPATCH ,
                                                       lOutParams[0]                   );
        aExecutor.start();
    }


    //_____________________

    /**
     * Internal call back for frame action events, triggered by the used
     * OnewayExecutor thread we started in frameAction().
     * We use it to update our interception on the internal saved frame.
     *
     * @param aEvent
     *          describes the action
     */
    public void impl_frameAction(/*IN*/ com.sun.star.frame.FrameActionEvent aEvent)
    {
        synchronized(this)
        {
            if (m_bDead)
                return;
        }

        // deregistration will be done everytime ...
        // But may it's not neccessary to establish a new registration!
        // Don't look for ignoring actions - it was done already inside original frameAction() call!
        boolean bRegister = false;

        // analyze the event and decide which reaction is usefull
        switch(aEvent.Action.getValue())
        {
            case com.sun.star.frame.FrameAction.COMPONENT_ATTACHED_value   : bRegister = true ; break;
            case com.sun.star.frame.FrameAction.COMPONENT_REATTACHED_value : bRegister = true ; break;
            case com.sun.star.frame.FrameAction.COMPONENT_DETACHING_value  : bRegister = false; break;
        }

        com.sun.star.frame.XFrame xFrame        = null ;
        boolean                   bIsRegistered = false;
        synchronized(this)
        {
            bIsRegistered   = m_bIsRegistered;
            m_bIsRegistered = false;
            xFrame          = m_xFrame;
        }

        com.sun.star.frame.XDispatchProviderInterception xRegistration = (com.sun.star.frame.XDispatchProviderInterception)UnoRuntime.queryInterface(
            com.sun.star.frame.XDispatchProviderInterception.class,
            xFrame);

        if(xRegistration==null)
            return;

        if (bIsRegistered)
            xRegistration.releaseDispatchProviderInterceptor(this);

        if (! bRegister)
            return;

        xRegistration.registerDispatchProviderInterceptor(this);
        synchronized(this)
        {
            m_bIsRegistered = true;
        }
    }

    // ____________________

    /**
     * Implementation of interface XDispatchProviderInterceptor
     * These functions are used to build a list of interceptor objects
     * connected in both ways.
     * Searching for a right interceptor is made by forwarding any request
     * from toppest master to lowest slave of this hierarchy.
     * If an interceptor whish to handle the request he can break that
     * and return himself as a dispatcher.
     */
    public com.sun.star.frame.XDispatchProvider getSlaveDispatchProvider()
    {
        synchronized(this)
        {
            return m_xSlave;
        }
    }

    // ____________________

    public void setSlaveDispatchProvider(com.sun.star.frame.XDispatchProvider xSlave)
    {
        synchronized(this)
        {
            m_xSlave = xSlave;
        }
    }

    // ____________________

    public com.sun.star.frame.XDispatchProvider getMasterDispatchProvider()
    {
        synchronized(this)
        {
            return m_xMaster;
        }
    }

    // ____________________

    public void setMasterDispatchProvider(com.sun.star.frame.XDispatchProvider xMaster)
    {
        synchronized(this)
        {
            m_xMaster = xMaster;
        }
    }

    // ____________________

    /**
     * Implementation of interface XDispatchProvider
     * These functions are called from our master if he willn't handle the outstanding request.
     * Given parameter should be checked if they are right for us. If it's true, the returned
     * dispatcher should be this implementation himself; otherwise call should be forwarded
     * to the slave.
     *
     * @param aURL
     *          describes the request, which should be handled
     *
     * @param sTarget
     *          specifies the target frame for this request
     *
     * @param nSearchFlags
     *          optional search flags, if sTarget isn't a special one
     *
     * @return [XDispatch]
     *          a dispatch object, which can handle the given URL
     *          May be NULL!
     */
    public com.sun.star.frame.XDispatch queryDispatch(/*IN*/ com.sun.star.util.URL aURL,/*IN*/ String sTarget,/*IN*/ int nSearchFlags)
    {
        synchronized(this)
        {
            if (m_bDead)
                return null;
        }

        // intercept loading empty documents into new created frames
        if(
            (sTarget.compareTo       ("_blank"         ) == 0   ) &&
            (aURL.Complete.startsWith("private:factory") == true)
          )
        {
            System.out.println("intercept private:factory");
            return this;
        }

        // intercept opening the SaveAs dialog
        if (aURL.Complete.startsWith(".uno:SaveAs") == true)
        {
            System.out.println("intercept SaveAs by returning null!");
            return null;
        }

        // intercept "File->Exit" inside the menu
        if (
            (aURL.Complete.startsWith("slot:5300") == true)  ||
            (aURL.Complete.startsWith(".uno:Quit") == true)
           )
        {
            System.out.println("intercept File->Exit");
            return this;
        }

        synchronized(this)
        {
            if (m_xSlave!=null)
                return m_xSlave.queryDispatch(aURL, sTarget, nSearchFlags);
        }

        return null;
    }

    // ____________________

    public com.sun.star.frame.XDispatch[] queryDispatches(/*IN*/ com.sun.star.frame.DispatchDescriptor[] lDescriptor)
    {
        synchronized(this)
        {
            if (m_bDead)
                return null;
        }
        // Resolve any request seperatly by using own "dispatch()" method.
        // Note: Don't pack return list if "null" objects occure!
        int                            nCount      = lDescriptor.length;
        com.sun.star.frame.XDispatch[] lDispatcher = new com.sun.star.frame.XDispatch[nCount];
        for(int i=0; i<nCount; ++i)
        {
            lDispatcher[i] = queryDispatch(lDescriptor[i].FeatureURL ,
                                           lDescriptor[i].FrameName  ,
                                           lDescriptor[i].SearchFlags);
        }
        return lDispatcher;
    }

    // ____________________

    /**
     * This method is called if this interceptor "wins the request".
     * We intercepted creation of new frames and loading of empty documents.
     * Do it now.
     *
     * @param aURL
     *          describes the document
     *
     * @param lArguments
     *          optional arguments for loading
     */
    public void impl_dispatch(/*IN*/ com.sun.star.util.URL aURL,/*IN*/ com.sun.star.beans.PropertyValue[] lArguments)
    {
        synchronized(this)
        {
            if (m_bDead)
                return;
        }

        if (
            (aURL.Complete.startsWith("slot:5300") == true) ||
            (aURL.Complete.startsWith(".uno:Quit") == true)
           )
        {
            System.exit(0);
        }
        else
        if (aURL.Complete.startsWith("private:factory") == true)
        {
            // Create view frame for showing loaded documents on demand.
            // The visible state is neccessary for JNI functionality to get the HWND and plug office
            // inside a java window hierarchy!
            DocumentView aNewView = new DocumentView();
            aNewView.setVisible(true);
            aNewView.createFrame();
            aNewView.load(aURL.Complete,lArguments);
        }
    }

    // ____________________

    /**
     * Notification of status listener isn't guaranteed (instead of listener on XNotifyingDispatch interface).
     * So this interceptor doesn't support that realy ...
     */
    public /*ONEWAY*/ void addStatusListener(/*IN*/ com.sun.star.frame.XStatusListener xListener,/*IN*/ com.sun.star.util.URL aURL)
    {
/*        if (aURL.Complete.startsWith(".uno:SaveAs")==true)
        {
            com.sun.star.frame.FeatureStateEvent aEvent = new com.sun.star.frame.FeatureStateEvent(
                                                                this,
                                                                aURL,
                                                                "",
                                                                false,
                                                                false,
                                                                null);
            if (xListener!=null)
            {
                System.out.println("interceptor disable SavAs by listener notify");
                xListener.statusChanged(aEvent);
            }
        }*/
    }

    // ____________________

    public /*ONEWAY*/ void removeStatusListener(/*IN*/ com.sun.star.frame.XStatusListener xListener,/*IN*/ com.sun.star.util.URL aURL)
    {
    }

    // ____________________

    /**
     * Implements (optional!) optimization for interceptor mechanism.
     * Any interceptor which provides this special interface is called automaticly
     * at registration time on this method. Returned URL's will be used to
     * call this interceptor directly without calling his masters before, IF(!)
     * following rules will be true:
     *      (1) every master supports this optional interface too
     *      (2) nobody of these masters whish to intercept same URL then this one
     * This interceptor whish to intercept creation of new documents.
     */
    public String[] getInterceptedURLs()
    {
        return INTERCEPTED_URLS;
    }

    // ____________________

    /**
     * This class listen on the intercepted frame to free all used resources on closing.
     * We forget the reference to the frame only here. Deregistration
     * isn't neccessary here - because this frame dies and wish to forgoten.
     *
     * @param aSource
     *          must be our internal saved frame, on which we listen for frame action events
     */
    public /*ONEAY*/ void disposing(/*IN*/ com.sun.star.lang.EventObject aSource)
    {
        synchronized(this)
        {
            if (m_bDead)
                return;
            if (m_xFrame!=null && UnoRuntime.areSame(aSource.Source,m_xFrame))
            {
                m_bIsActionListener = false;
                m_xFrame            = null ;
            }
        }
        shutdown();
    }

    // ____________________

    /**
     * If this java application shutdown - we must cancel all current existing
     * listener connections. Otherwhise the office will run into some
     * DisposedExceptions if it tries to use these forgotten listener references.
     * And of course it can die doing that.
     * We are registered at a central object to be informed if the VM will exit.
     * So we can react.
     */
    public void shutdown()
    {
        com.sun.star.frame.XFrame xFrame            = null ;
        boolean                   bIsRegistered     = false;
        boolean                   bIsActionListener = false;
        synchronized(this)
        {
            // don't react a second time here!
            if (m_bDead)
                return;
            m_bDead = true;

            bIsRegistered       = m_bIsRegistered;
            m_bIsRegistered     = false;

            bIsActionListener   = m_bIsActionListener;
            m_bIsActionListener = false;

            xFrame              = m_xFrame;
            m_xFrame            = null;
        }

        // it's a good idead to cancel listening for frame action events
        // before(!) we deregister us as an interceptor.
        // Because registration and deregistratio nof interceptor objects
        // will force sending of frame action events ...!
        if (bIsActionListener)
            xFrame.removeFrameActionListener(this);

        if (bIsRegistered)
        {
            com.sun.star.frame.XDispatchProviderInterception xRegistration = (com.sun.star.frame.XDispatchProviderInterception)UnoRuntime.queryInterface(
                com.sun.star.frame.XDispatchProviderInterception.class,
                xFrame);

            if(xRegistration!=null)
                xRegistration.releaseDispatchProviderInterceptor(this);
        }

        xFrame = null;

        synchronized(this)
        {
            m_xMaster = null;
            m_xSlave  = null;
        }
    }
}

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

import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.task.*;
import com.sun.star.uno.*;

/** it implements a simple job component.
 *
 *  Such jobs are executable in different ways:
 *  <ul>
 *      <li>registered for a special URL schema "vnd.sun.star.jobs:*" and used from the generic dispatch framework</li>
 *      <li>the global com.sun.star.task.JobExecutor service and registered for special events.</li>
 *  </ul>
 */
public class AsyncJob extends    WeakBase implements XServiceInfo, XAsyncJob
{
    public final XComponentContext m_xCmpCtx;

    /** the const list of supported uno service names. */
    public static final java.lang.String[] SERVICENAMES = {"com.sun.star.task.AsyncJob"};

    /** the const uno implementation name.
     *  It must be an unique value! The best naming schema seams to use
     *  a registered domain in reverse order ...
     */
    public static final java.lang.String IMPLEMENTATIONNAME = "com.sun.star.comp.framework.java.services.AsyncJob";

    //___________________________________________
    // interface

    /** initialize a new instance of this class with default values. */
    public AsyncJob( XComponentContext xCompContext )
    {
        m_xCmpCtx = xCompContext;
    }

    //___________________________________________

    /** starts execution of this job.
     *
     *  @param  lArgs
     *          list which contains:
     *          <ul>
     *              <li>generic job configuration data</li>
     *              <li>job specific configuration data</li>
     *              <li>some environment information</li>
     *              <li>may optional arguments of a corresponding dispatch request</li>
     *          </ul>
     *
     *  @params xListener
     *          callback to the executor of this job, which control our life time
     *
     *  @throws com.sun.star.lang.IllegalArgumentException
     *          if given argument list seams to be wrong
     */
    public synchronized void executeAsync(com.sun.star.beans.NamedValue[] lArgs    ,
                                          com.sun.star.task.XJobListener  xListener)
        throws com.sun.star.lang.IllegalArgumentException
    {
        // For asynchronous jobs a valid listener reference is guranteed normaly ...
        if (xListener == null)
            throw new com.sun.star.lang.IllegalArgumentException("invalid listener");

        // extract all possible sub list of given argument list
        com.sun.star.beans.NamedValue[] lGenericConfig = null;
        com.sun.star.beans.NamedValue[] lJobConfig     = null;
        com.sun.star.beans.NamedValue[] lEnvironment   = null;
        com.sun.star.beans.NamedValue[] lDynamicData   = null;

        int c = lArgs.length;
        for (int i=0; i<c; ++i)
        {
            if (lArgs[i].Name.equals("Config"))
                lGenericConfig = (com.sun.star.beans.NamedValue[])com.sun.star.uno.AnyConverter.toArray(lArgs[i].Value);
            else
            if (lArgs[i].Name.equals("JobConfig"))
                lJobConfig = (com.sun.star.beans.NamedValue[])com.sun.star.uno.AnyConverter.toArray(lArgs[i].Value);
            else
            if (lArgs[i].Name.equals("Environment"))
                lEnvironment = (com.sun.star.beans.NamedValue[])com.sun.star.uno.AnyConverter.toArray(lArgs[i].Value);
            else
            if (lArgs[i].Name.equals("DynamicData"))
                lDynamicData = (com.sun.star.beans.NamedValue[])com.sun.star.uno.AnyConverter.toArray(lArgs[i].Value);
        }

        // Analyze the environment info. This sub list is the only guarenteed one!
        if (lEnvironment == null)
            throw new com.sun.star.lang.IllegalArgumentException("no environment");

        java.lang.String          sEnvType   = null;
        java.lang.String          sEventName = null;
        com.sun.star.frame.XFrame xFrame     = null;
        c = lEnvironment.length;
        for (int i=0; i<c; ++i)
        {
            if (lEnvironment[i].Name.equals("EnvType"))
                sEnvType = com.sun.star.uno.AnyConverter.toString(lEnvironment[i].Value);
            else
            if (lEnvironment[i].Name.equals("EventName"))
                sEventName = com.sun.star.uno.AnyConverter.toString(lEnvironment[i].Value);
            else
            if (lEnvironment[i].Name.equals("Frame"))
                xFrame = (com.sun.star.frame.XFrame)com.sun.star.uno.AnyConverter.toObject(
                            new com.sun.star.uno.Type(com.sun.star.frame.XFrame.class),
                            lEnvironment[i].Value);
        }

        // Further the environment property "EnvType" is required as minimum.
        if (
            (sEnvType==null) ||
            (
             (!sEnvType.equals("EXECUTOR")) &&
             (!sEnvType.equals("DISPATCH"))
            )
           )
        {
            java.lang.String sMessage = "\"" + sEnvType + "\" isn't a valid value for EnvType";
            throw new com.sun.star.lang.IllegalArgumentException(sMessage);
        }

        // Analyze the set of shared config data.
        java.lang.String sAlias = null;
        if (lGenericConfig!=null)
        {
            c = lGenericConfig.length;
            for (int i=0; i<c; ++i)
            {
                if (lGenericConfig[i].Name.equals("Alias"))
                    sAlias = com.sun.star.uno.AnyConverter.toString(lGenericConfig[i].Value);
            }
        }

        // do your job ...
        // Here we print out all found arguments.
        java.lang.String sOut = formatOutArgs(lGenericConfig, lJobConfig, lEnvironment, lDynamicData);
        if (xFrame != null)
            showInfoModal(xFrame.getContainerWindow(), "Arguments of AsyncJob initialization ...", sOut);
        else
            showInfoNonModal("Arguments of AsyncJob initialization ...", sOut);

        // use return value to start different actions
        // But look for the right environment. Some options make no sense inside the wrong env.
        com.sun.star.beans.NamedValue aDeactivation   = null;
        com.sun.star.beans.NamedValue aDispatchResult = null;
        com.sun.star.beans.NamedValue aSaveRequest    = null;

        // SaveArguments will be made everytimes!
        c = 1;

        if (lJobConfig==null)
            lJobConfig = new com.sun.star.beans.NamedValue[1];
        lJobConfig[0] = new com.sun.star.beans.NamedValue();
        lJobConfig[0].Name  = "arg_1";
        lJobConfig[0].Value = "val_1";

        aSaveRequest = new com.sun.star.beans.NamedValue();
        aSaveRequest.Name  = "SaveArguments";
        aSaveRequest.Value = lJobConfig;

        // Deactivation is useful inside EXECUTOR environment only
        if (sEnvType.equals("EXECUTOR"))
        {
            ++c;
            aDeactivation       = new com.sun.star.beans.NamedValue();
            aDeactivation.Name  = "Deactivate";
            aDeactivation.Value = java.lang.Boolean.TRUE;
        }

        // Sending of result events is useful inside DISPATCH environment only
        if (sEnvType.equals("DISPATCH"))
        {
            ++c;
            aDispatchResult       = new com.sun.star.beans.NamedValue();
            aDispatchResult.Name  = "SendDispatchResult";
            aDispatchResult.Value = new com.sun.star.frame.DispatchResultEvent(this, com.sun.star.frame.DispatchResultState.SUCCESS, null);
        }

        // pack it together for return
        int i=0;
        com.sun.star.beans.NamedValue[] lReturn = new com.sun.star.beans.NamedValue[c];
        lReturn[i++] = aSaveRequest;
        if (aDeactivation!=null)
            lReturn[i++] = aDeactivation;
        if (aDispatchResult!=null)
            lReturn[i++] = aDispatchResult;

        xListener.jobFinished(this, lReturn);
    }

    //___________________________________________

    /** show an info box with the UNO based toolkit.
     *
     *  It tries to use the container window of a may well know
     *  office frame as parent. If such parent window could be located,
     *  the info box can be shown in modal mode. If a parent is missing
     *  (because this job is called inside an EXECUTOR environment, which
     *  does not set any frame context here) the info box can't be created!
     *  Because the toolkit needs parents for non top level windows ...
     *  In that case the only way is to implement this info box
     *  native or make it non modal using java dialogs inside it's own thread ...
     *  (see showInfoNonModal() too)
     *
     *  @param  xParent
     *          used as parent window of the shown info box.
     *
     *  @param  sTitle
     *          is shown as title of the info box.
     *
     *  @param  sMessage
     *          inclused the message body, which is shown as info.
     */

    private void showInfoModal( com.sun.star.awt.XWindow xParent  ,
                                java.lang.String         sTitle   ,
                                java.lang.String         sMessage )
    {
        try
        {
            // get access to the office toolkit environment
            com.sun.star.awt.XToolkit xKit = UnoRuntime.queryInterface(
                 com.sun.star.awt.XToolkit.class,
                 m_xCmpCtx.getServiceManager().createInstanceWithContext("com.sun.star.awt.Toolkit",
                                                                         m_xCmpCtx));

            // describe the info box ini it's parameters
            com.sun.star.awt.WindowDescriptor aDescriptor = new com.sun.star.awt.WindowDescriptor();
            aDescriptor.WindowServiceName = "infobox";
            aDescriptor.Bounds            = new com.sun.star.awt.Rectangle(0,0,300,200);
            aDescriptor.WindowAttributes  = com.sun.star.awt.WindowAttribute.BORDER   |
                                            com.sun.star.awt.WindowAttribute.MOVEABLE |
                                            com.sun.star.awt.WindowAttribute.CLOSEABLE;
            aDescriptor.Type              = com.sun.star.awt.WindowClass.MODALTOP;
            aDescriptor.ParentIndex       = 1;
            aDescriptor.Parent            = UnoRuntime.queryInterface(
                                                com.sun.star.awt.XWindowPeer.class,
                                                xParent);

            // create the info box window
            com.sun.star.awt.XWindowPeer xPeer    = xKit.createWindow(aDescriptor);
            com.sun.star.awt.XMessageBox xInfoBox = UnoRuntime.queryInterface(
                                                        com.sun.star.awt.XMessageBox.class,
                                                        xPeer);
            if (xInfoBox == null)
                return;

            // fill it with all given information and show it
            xInfoBox.setCaptionText(sTitle);
            xInfoBox.setMessageText(sMessage);
            xInfoBox.execute();
        }
        catch(java.lang.Throwable exIgnore)
        {
            // ignore any problem, which can occure here.
            // It's not realy a bug for this example job, if
            // it's message could not be printed out!
        }
    }

    //___________________________________________

    private void showInfoNonModal( java.lang.String sTitle   ,
                                   java.lang.String sMessage )
    {
        // Couldnt be implemented realy using the toolkit ...
        // Because we need a parent anytime.
        // And showing e.g. a java dialog can make some trouble
        // inside office ... but we have no chance here.
    final java.lang.String sFinalTitle = sTitle;
    final java.lang.String sFinalMessage = sMessage;

    // On Mac OS X, AWT/Swing must not be accessed from the AppKit thread, so call
    // SwingUtilities.invokeLater always on a fresh thread to avoid that problem
    // (also, the current thread must not wait for that fresh thread to terminate,
    // as that would cause a deadlock if this thread is the AppKit thread):
    final Runnable doRun = new Runnable() {
        public void run() {
            javax.swing.JOptionPane.showMessageDialog(null, sFinalMessage, sFinalTitle, javax.swing.JOptionPane.INFORMATION_MESSAGE);
        }
        };

     new Thread( doRun ) {
         public void run() { javax.swing.SwingUtilities.invokeLater(doRun); }
     }.start();
    }

    //___________________________________________

    /** helper to print out the given argument list.
     *
     *  @param  lGenericConfig
     *          contains all shared configuration items for a job
     *
     *  @param  lJobConfig
     *          contains all job sepcific configuration items
     *
     *  @param  lEnvironment
     *          contains some environment information
     *
     *  @param  lDynamicData
     *          contains optional data of a might corresponding dispatch() request
     */

    private java.lang.String formatOutArgs(com.sun.star.beans.NamedValue[] lGenericConfig,
                                           com.sun.star.beans.NamedValue[] lJobConfig    ,
                                           com.sun.star.beans.NamedValue[] lEnvironment  ,
                                           com.sun.star.beans.NamedValue[] lDynamicData )
    {
        java.lang.StringBuffer sOut = new java.lang.StringBuffer(1024);

        sOut.append("list \"Config\": ");
        if (lGenericConfig==null)
            sOut.append("0 items\n");
        else
        {
            int c = lGenericConfig.length;
            sOut.append(c+" items\n");
            for (int i=0; i<c; ++i)
                sOut.append("\t["+i+"] \""+lGenericConfig[i].Name+"\" = {"+lGenericConfig[i].Value+"}\n");
        }
        sOut.append("list \"JobConfig\": ");
        if (lJobConfig==null)
            sOut.append("0 items\n");
        else
        {
            int c = lJobConfig.length;
            sOut.append(c+" items\n");
            for (int i=0; i<c; ++i)
                sOut.append("\t["+i+"] \""+lJobConfig[i].Name+"\" = {"+lJobConfig[i].Value+"}\n");
        }
        sOut.append("list \"Environment\": ");
        if (lEnvironment==null)
            sOut.append("0 items\n");
        else
        {
            int c = lEnvironment.length;
            sOut.append(c+" items\n");
            for (int i=0; i<c; ++i)
                sOut.append("\t["+i+"] \""+lEnvironment[i].Name+"\" = {"+lEnvironment[i].Value+"}\n");
        }
        sOut.append("list \"DynamicData\": ");
        if (lDynamicData==null)
            sOut.append("0 items\n");
        else
        {
            int c = lDynamicData.length;
            sOut.append(c+" items\n");
            for (int i=0; i<c; ++i)
                sOut.append("\t["+i+"] \""+lDynamicData[i].Name+"\" = {"+lDynamicData[i].Value+"}\n");
        }

        return sOut.toString();
    }

    public String[] getSupportedServiceNames() {
        return SERVICENAMES;
    }

    public boolean supportsService( String sService ) {
        int len = SERVICENAMES.length;

        for( int i=0; i < len; i++) {
            if ( sService.equals( SERVICENAMES[i] ) )
                return true;
        }

        return false;
    }

    public String getImplementationName() {
        return( AsyncJob.class.getName() );
    }


    //___________________________________________

    public synchronized static com.sun.star.lang.XSingleComponentFactory __getComponentFactory(java.lang.String sImplName)
    {
        com.sun.star.lang.XSingleComponentFactory xFactory = null;
        if (sImplName.equals(AsyncJob.IMPLEMENTATIONNAME))
            xFactory = Factory.createComponentFactory(AsyncJob.class, SERVICENAMES);

        return xFactory;
    }

    //___________________________________________
    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.openoffice.org/wiki/Passive_Component_Registration

//     public synchronized static boolean __writeRegistryServiceInfo(com.sun.star.registry.XRegistryKey xRegKey)
//     {
//         return Factory.writeRegistryServiceInfo(
//             AsyncJob.IMPLEMENTATIONNAME,
//             AsyncJob.SERVICENAMES,
//             xRegKey);
//     }
}

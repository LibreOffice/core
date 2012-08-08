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

package OfficeDev.samples.Filter;

import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;
import com.sun.star.uno.AnyConverter;
import com.sun.star.beans.PropertyValue;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.container.XNamed;
import com.sun.star.document.XImporter;
import com.sun.star.document.XExporter;
import com.sun.star.document.XFilter;


import com.sun.star.lang.IllegalArgumentException;

/*-************************************************************************
  @title        implements a filter to import pure ascii text files
  @description  This filter can use an existing In/OutputStream of given
                MediaDescriptor or use an existing URL instead of that
                to open the file directly. But for second case the local
                file system will be used only. There is no support for remote.

                During import/export special functionality can be used to
                e.g. to change some characters inside the file content
                or replace some strings (no using of regular expressions!).
                User can decide at runtime which functionality realy should
                be used by selecting it in an extra filter property dialog.

                So we show how a filter works fro iport/export, use or create
                streams and how a filter can offer properties for filtering
                which can be edit by the user.
 ************************************************************************-*/

public class AsciiReplaceFilter
{
    public static class _AsciiReplaceFilter extends     WeakBase
                                            implements  XInitialization ,
                                                        XServiceInfo    ,
                                                        XNamed          ,
                                                        XImporter       ,
                                                        XExporter       ,
                                                        XFilter
    {


        // the supported service names, the first one being the service name of the component itself
        public static final String[] m_serviceNames = { "com.sun.star.comp.ansifilter.AsciiReplaceFilter" , "com.sun.star.document.ImportFilter", "com.sun.star.document.ExportFilter" };

        // filterprocess states
        public static final int    FILTERPROC_RUNS          = 0;
        public static final int    FILTERPROC_BREAK         = 1;
        public static final int    FILTERPROC_STOPPED       = 2;

        //______________________________
        // member


        /// The initial component context, that gives access to the service manager, supported singletons, ...
        private XComponentContext m_Ctx;
        /// The service manager, that gives access to all registered services and which is passed to the FilterOptions class for instantiating a ucb service
        private XMultiComponentFactory m_xMCF;
        /// we must provide our name
        private String m_sInternalName;
        /// saved document reference for import or export (depends on other member m_bImport!)
        private com.sun.star.text.XTextDocument m_xDocument;
        /// because we implement an import AND an export filter, we must know which one is required
        private boolean m_bImport;
        // we need a flag to cancel any running filter operation
        private int m_nFilterProcState;

        //______________________________
        // native interface
       /**
        * special debug helper to get an idea how expensive
        * the implemented filter operations are realy.
        * May be usefully for own purposes.
        *
        * To see the output inside an office environment
        * use "soffice ...params... >output.txt"
        */
        private long m_nStart;
        private long m_nLast ;

        private void measure( String sText )
        {
            long nNow = System.currentTimeMillis();
            System.err.println(sText+"\t"+(nNow-m_nStart)+"\t"+(nNow-m_nLast));
            m_nLast = nNow;
        }

        //______________________________
        // native interface
        /**
        * The constructor to initialize every instance
        *
        * @param xCompContext
        *  the component context of the office
        */
        //ctor
        public _AsciiReplaceFilter(XComponentContext Context )
        {
            measure("ctor started");
            try
            {
                m_Ctx = Context                     ;
                m_xMCF = m_Ctx.getServiceManager()  ;
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }

            // these are safe, thus no errorhandling needed:
            m_sInternalName    = new String()       ;
            m_xDocument        = null               ;
            m_bImport          = true               ;
            m_nFilterProcState = FILTERPROC_STOPPED ;

            m_nLast  = System.currentTimeMillis();
            m_nStart = m_nLast;
        }

        //______________________________
        // interface XInitialization
        /**
        * used for initializing after creation
        * If an instance of this service is created by UNO we will be called
        * automaticly after that to get optional parameters of this creation call.
        * E.g.: The service com.sun.star.document.FilterFactory use such mechanism
        * to pass our own configuration data to this instance.
        *
        * @param lArguments
        *  This array of arbitrary objects represent our own filter configuration
        *  and may optional given parameters of the createWithArguments() call.
        *
        * @throws Exception
        *  Every exception will not be handled, but will be
        *  passed to the caller.
        */
        public void initialize( Object[] lArguments ) throws com.sun.star.uno.Exception
        {
            measure("initialize {");

            if (lArguments.length<1)
                return;

            // lArguments[0] = own configuration data
            com.sun.star.beans.PropertyValue[] lConfig = (com.sun.star.beans.PropertyValue[])lArguments[0];

            /*
            // lArguments[1..n] = optional arguments of create request
            for (int n=1; n<lArguments.length; ++n)
            {
            }
            */

            // analyze own configuration data for our own internal filter name!
            // Important for generic filter services, which are registered more then once.
            // They can use this information to find out, which specialization of it
            // is required.
            for (int i=0; i<lConfig.length; ++i)
            {
                if (lConfig[i].Name.equals("Name"))
                {
                    synchronized(this)
                    {
                        try
                        {
                            m_sInternalName = AnyConverter.toString(lConfig[i].Value);
                        }
                        catch(com.sun.star.lang.IllegalArgumentException exConvert) {}
                    }
                }
            }

            measure("} initialize");
        }

        //______________________________
        // interface XNamed
        /**
        * For external user of us we must provide our internal filter name
        * (which is registered inside configuration package TypeDetection).
        * User will be able then to ask there for furthe information about us.
        * Otherwhise we must implement a full featured XPropertySet ...
        *
        * @return our internal filter name of configuration
        */
        public String getName()
        {
            synchronized(this)
            {
                return m_sInternalName;
            }
        }

        /**
        * It's not allowed for us - neither very easy to change our internal
        * name during runtime of an office. Because every filter name must
        * be unambigous ...
        * So we doesn't implement this method here.
        */
        public void setName( String sName )
        {
        }

        //______________________________
        // interface XImporter
        /**
        * This interface is used to tell us: "you will be used for importing a document".
        * We must save the given model reference to use it inside our own filter request.
        *
        * @param xDocument
        *          the document model for importing
        *
        * @throw IllegalArgumentException
        *          if given document isn't the right one or seams to be corrupt
        */
        public void setTargetDocument( com.sun.star.lang.XComponent xDocument ) throws com.sun.star.lang.IllegalArgumentException
        {
            measure("setTargetDocument {");

            if (xDocument==null)
                throw new com.sun.star.lang.IllegalArgumentException("null reference detected");

            com.sun.star.lang.XServiceInfo xInfo = (com.sun.star.lang.XServiceInfo)UnoRuntime.queryInterface(
                                    com.sun.star.lang.XServiceInfo.class, xDocument);
            if ( ! xInfo.supportsService("com.sun.star.text.TextDocument") )
                throw new com.sun.star.lang.IllegalArgumentException( "wrong document type" );

            // safe it as target document for import
            // Don't forget to mark this filter used for importing too
            synchronized(this)
            {
                m_xDocument = (com.sun.star.text.XTextDocument)UnoRuntime.queryInterface(
                        com.sun.star.text.XTextDocument.class, xDocument);
                m_bImport = true;
            }

            measure("} setTargetDocument");
        }

        //______________________________
        // interface XExporter
        /**
        * This interface is used to tell us: "you will be used for exporting a document".
        * We must save the given model reference to use it inside our own filter request.
        *
        * @param xDocument
        *          the document model for exporting
        *
        * @throw IllegalArgumentException
        *          if given document isn't the right one or seams to be corrupt
        */
        public void setSourceDocument( com.sun.star.lang.XComponent xDocument ) throws com.sun.star.lang.IllegalArgumentException
        {
            measure("setSourceDocument {");

            if (xDocument==null)
                throw new com.sun.star.lang.IllegalArgumentException( "null reference given" );

            com.sun.star.lang.XServiceInfo xInfo = (com.sun.star.lang.XServiceInfo)UnoRuntime.queryInterface(
                        com.sun.star.lang.XServiceInfo.class, xDocument);
            if ( ! xInfo.supportsService("com.sun.star.text.TextDocument") )
                throw new com.sun.star.lang.IllegalArgumentException( "wrong document type" );

            // safe it as source document for export
            // Don't forget to mark this filter used for exporting too
            synchronized(this)
            {
                m_xDocument = (com.sun.star.text.XTextDocument)UnoRuntime.queryInterface(
                        com.sun.star.text.XTextDocument.class, xDocument);
                m_bImport = false;
            }

            measure("} setSourceDocument");
        }


        //______________________________
        // interface XFilter
        /**
        * Implements the real filter method. We detect if it must be an import or an export.
        * Depends on that we use an existing stream (given inside the MediaDescriptor)
        * or open it by using an URL (must be a part of the descriptor too).
        *
        * @param lDescriptor
        *          the MediaDescriptor which describes the document
        *
        * @return a bool value which describes if method was successfully or not.
        */

        public boolean filter( com.sun.star.beans.PropertyValue[] lDescriptor )
        {
            measure("filter {");

            // first get state of filter operation (import/export)
            // and try to create or get corresponding streams
            // Means: analyze given MediaDescriptor
            // By the way: use synchronized section to get some copies of other
            // internal states too.
            FilterOptions                   aOptions = null ;
            boolean                         bImport  = false;
            com.sun.star.text.XTextDocument xText    = null ;
            synchronized(this)
            {
                aOptions = new FilterOptions(m_xMCF, m_Ctx, m_bImport, lDescriptor);
                bImport  = m_bImport;
                xText    = m_xDocument;
            }

            measure("options analyzed");

            if (aOptions.isValid()==false)
                return false;

            // start real filtering
            boolean bState = false;
            if (bImport)
                bState = implts_import( xText, aOptions );
            else
                bState = implts_export( xText, aOptions );

            measure("} filter");

            return bState;
        }

        /**
        * Makes the filter process breakable. To do so the outside code may use threads.
        * We use a internal "condition" variable wich is queried by the real filter method on
        * every loop they do. So it's more a polling mechanism.
        */
        public void cancel()
        {
            measure("cancel {");

            synchronized(this)
            {
                if (m_nFilterProcState==FILTERPROC_RUNS)
                    m_nFilterProcState=FILTERPROC_BREAK;
            }

            while (true)
            {
                synchronized(this)
                {
                    if (m_nFilterProcState==FILTERPROC_STOPPED)
                        break;
                }
            }

            measure("} cancel");
        }

        //______________________________
        // private helper
        /**
         * This helper function imports a simple ascii text file into
         * a text model. We copy every letter to the document.
         * But if some optional filter options are given
         * we make some changes: replace chars or complete strings.
         *
         * Note: It's not alloed for a filter to seek inside the stream.
         * Because the outside frameloader has to set the stream position
         * right and a filter must read till EOF occures only.
         *
         * @param xTarget
         *          the target text model to put the data in
         *
         * @param aOptions
         *          capsulate all other neccessary informations for this filter request
         *          (streams, replace values ...)
         *
         * @return a bool value which describes if method was successfully or not.
         */
        private boolean implts_import( com.sun.star.text.XTextDocument xTarget  ,
                                       FilterOptions                   aOptions )
        {
            measure("implts_import {");

            com.sun.star.text.XSimpleText xText = (com.sun.star.text.XSimpleText)UnoRuntime.queryInterface(
                com.sun.star.text.XSimpleText.class,
                xTarget.getText());

            measure("cast XSimpleText");

            boolean bBreaked = false;

            try
            {
                StringBuffer sBuffer  = new StringBuffer(100000);
                byte[][]     lData    = new byte[1][];
                int          nRead    = aOptions.m_xInput.readBytes( lData, 4096 );

                measure("read first bytes");

                while (nRead>0 && !bBreaked)
                {
                    // copy data from stream to temp. buffer
                    sBuffer.append( new String(lData[0]) );
                    measure("buffer append ["+nRead+"]");

                    nRead = aOptions.m_xInput.readBytes( lData, 2048 );
                    measure("read next bytes");

                    // check for cancelled filter proc on every loop!
                    synchronized(this)
                    {
                        if (m_nFilterProcState==FILTERPROC_BREAK)
                        {
                            m_nFilterProcState = FILTERPROC_STOPPED;
                            return false;
                        }
                    }
                    measure("break check");
                }

                // Make some replacements inside the buffer.
                String sText = implts_replace( sBuffer, aOptions );
                measure("replace");

                // copy current buffer to the document model.
                // Create a new paragraph for every line inside original file.
                // May not all data could be readed - but that doesn't matter here.
                // Reason: somewhere cancelled this function.
                // But check for optioanl replace request before ...
                int nStart  =  0;
                int nEnd    = -1;
                int nLength = sText.length();

                com.sun.star.text.XTextRange xCursor = (com.sun.star.text.XTextRange)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextRange.class,
                    xText.createTextCursor());

                while (true)
                {
                    nEnd = sText.indexOf('\n',nStart);

                    if (nEnd==-1 && nStart<nLength)
                        nEnd = nLength;

                    if (nEnd==-1)
                        break;

                    String sLine = sText.substring(nStart,nEnd);
                    nStart = nEnd+1;

                    xText.insertString(xCursor,sLine,false);
                    xText.insertControlCharacter(xCursor,com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK,false);

                    // check for cancelled filter proc on every loop!
                    synchronized(this)
                    {
                        if (m_nFilterProcState==FILTERPROC_BREAK)
                        {
                            m_nFilterProcState = FILTERPROC_STOPPED;
                            return false;
                        }
                    }
                    measure("break check");
                }

                measure("set on model");

                // with refreshing the document we are on the safe-side, otherwise the first time the filter is used the document is not fully shown (flaw!).
                com.sun.star.util.XRefreshable xRefresh = (com.sun.star.util.XRefreshable)UnoRuntime.queryInterface(
                com.sun.star.util.XRefreshable.class,
                xTarget);
                xRefresh.refresh();

                // If we created used stream - we must close it too.
                if (aOptions.m_bStreamOwner==true)
                {
                    aOptions.m_xInput.closeInput();
                    measure("stream close");
                }
            }
            catch(com.sun.star.lang.IllegalArgumentException  exArgument ) { bBreaked = true; }
            catch(com.sun.star.io.BufferSizeExceededException exExceed   ) { bBreaked = true; }
            catch(com.sun.star.io.NotConnectedException       exConnect  ) { bBreaked = true; }
            catch(com.sun.star.io.IOException                 exIO       ) { bBreaked = true; }



            measure("} implts_import");

            return !bBreaked;
        }

        /**
         * This helper function exports a simple ansi text file from
         * a text model. We copy every letter from the document.
         * There are no checks.
         *
         * Note: It's not alloed for a filter to seek inside the stream.
         * Because the outside frameloader has to set the stream position
         * right and a filter must read till EOF occures only.
         *
         * @param xSource
         *          the source text model to get the data from
         *
         * @param aOptions
         *          capsulate all other neccessary informations for this filter request
         *          (streams, replace values ...)
         *
         * @return a bool value which describes if method was successfully or not.
         */
        private boolean implts_export( com.sun.star.text.XTextDocument xSource ,
                                       FilterOptions                   aOptions)
        {
            measure("implts_export {");

            com.sun.star.text.XTextRange xText = (com.sun.star.text.XSimpleText)UnoRuntime.queryInterface(
                com.sun.star.text.XSimpleText.class,
                xSource.getText());

            measure("cast XTextRange");

            boolean bBreaked = false;

            try
            {
                StringBuffer sBuffer = new StringBuffer(xText.getString());
                String       sText   = implts_replace(sBuffer,aOptions);

                measure("get text from model");

                // Normaly this function isn't realy cancelable
                // But we following operation can be very expensive. So
                // this place is the last one to stop it.
                synchronized(this)
                {
                    if (m_nFilterProcState==FILTERPROC_BREAK)
                    {
                        m_nFilterProcState = FILTERPROC_STOPPED;
                        return false;
                    }
                }

                aOptions.m_xOutput.writeBytes(sText.getBytes());
                aOptions.m_xOutput.flush();

                measure("written to file");

                // If we created used stream - we must close it too.
                if (aOptions.m_bStreamOwner==true)
                {
                    aOptions.m_xOutput.closeOutput();
                    measure("stream close");
                }
            }
            catch(com.sun.star.io.BufferSizeExceededException exExceed  ) { bBreaked = true; }
            catch(com.sun.star.io.NotConnectedException       exConnect ) { bBreaked = true; }
            catch(com.sun.star.io.IOException                 exIO      ) { bBreaked = true; }

            measure("} implts_export");

            return !bBreaked;
        }

        /**
         * helper function to convert the used StringBuffer into a Strig value.
         * And we use this chance to have a look on optional filter options
         * which can invite replacing of strings.
         */
        private String implts_replace( StringBuffer rBuffer, FilterOptions aOptions )
        {
            // replace complete strings first
            // Because its easiear on a buffer then on a string
            if ( ! aOptions.m_sOld.equals(aOptions.m_sNew) )
            {
                int nStart  = rBuffer.indexOf(aOptions.m_sOld);
                int nLength = aOptions.m_sNew.length();
                int nEnd    = nStart+nLength;
                while (nStart!=-1)
                {
                    rBuffer.replace(nStart,nEnd,aOptions.m_sNew);
                    nStart = rBuffer.indexOf(aOptions.m_sOld,nEnd);
                    nEnd   = nStart+nLength;
                }
            }

            // convert buffer into return format [string]
            // and convert to lower or upper case if required.
            String sResult = rBuffer.toString();
            if (aOptions.m_bCaseChange==true)
            {
                if (aOptions.m_bLower==true)
                    sResult = sResult.toLowerCase();
                else
                    sResult = sResult.toUpperCase();
            }

            return sResult;
        }


        //______________________________
        // interface XServiceInfo
        /**
         * This method returns an array of all supported service names.
         *
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames()
        {
             return m_serviceNames;
        }

        /**
         * This method returns true, if the given service will be
         * supported by this component.
         *
         * @param sService
         *  the requested service name
         *
         * @return True, if the given service name will be supported;
         *         False otherwhise.
         */
        public boolean supportsService( String sService )
        {
            return (
                    sService.equals( m_serviceNames[0] ) ||
                    sService.equals( m_serviceNames[1] ) ||
                    sService.equals( m_serviceNames[2] )
                    );
        }

        /**
         * Return the real class name of the component
         *
         * @return Class name of the component.
         */
        public String getImplementationName()
        {
            return _AsciiReplaceFilter.class.getName();
        }


    }
    // end of inner class, the wrapper class just has the two methods required for registering the component

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     *
     * @param sImplName
     *  The implementation name of the component.
     *
     * @return Returns a <code>XSingleComponentFactory</code> for
     *         creating the component.
     *
     * @see com.sun.star.comp.loader.JavaLoader
     */

    public static XSingleComponentFactory __getComponentFactory(String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( _AsciiReplaceFilter.class.getName() ) )
            xFactory = com.sun.star.lib.uno.helper.Factory.createComponentFactory(_AsciiReplaceFilter.class,
                                             _AsciiReplaceFilter.m_serviceNames);
        return xFactory;
    }


    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>.
     *
     * @param xRegistryKey
     *  Makes structural information (except regarding tree
     *  structures) of a single registry key accessible.
     *
     * @return returns true if the operation succeeded
     *
     * @see com.sun.star.comp.loader.JavaLoader
     */
    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.services.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo( com.sun.star.registry.XRegistryKey xRegistryKey )
//     {
//         return Factory.writeRegistryServiceInfo(
//             _AsciiReplaceFilter.class.getName(),
//             _AsciiReplaceFilter.m_serviceNames,
//             xRegistryKey );
//     }
}

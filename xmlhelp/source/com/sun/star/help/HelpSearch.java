/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package com.sun.star.help;

import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.XComponentContext;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.analysis.cjk.CJKAnalyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.Term;
import org.apache.lucene.search.TopDocs;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Searcher;
import org.apache.lucene.search.ScoreDoc;
import org.apache.lucene.search.TermQuery;
import org.apache.lucene.search.WildcardQuery;
import org.apache.lucene.util.Version;
import org.apache.lucene.store.NIOFSDirectory;

import com.sun.star.script.XInvocation;
import com.sun.star.beans.XIntrospectionAccess;

import java.io.File;

/** This class capsulates the class, that implements the minimal component and a
 * factory for creating the service (<CODE>__getComponentFactory</CODE>).
 */
public class HelpSearch
{
    /** This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class _HelpSearch extends WeakBase
        implements XServiceInfo, XInvocation
    {
        /** The service name, that must be used to get an instance of this service.
         */
        static private final String __serviceName =
            "com.sun.star.help.HelpSearch";
        static private final String aSearchMethodName = "search";

        /** The initial component contextr, that gives access to
         * the service manager, supported singletons, ...
         * It's often later used
         */
        private XComponentContext m_cmpCtx;

        /** The service manager, that gives access to all registered services.
         * It's often later used
         */
        private XMultiComponentFactory m_xMCF;

        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param xmultiservicefactoryInitialization A special service factory
         * could be introduced while initializing.
         */
        public _HelpSearch(XComponentContext xCompContext)
        {
            try {
                m_cmpCtx = xCompContext;
                m_xMCF = m_cmpCtx.getServiceManager();
            }
            catch( Exception e ) {
                e.printStackTrace();
            }
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames()
        {
            return getServiceNames();
        }

        /** This method is a simple helper function to used in the
         * static component initialisation functions as well as in
         * getSupportedServiceNames.
         */
        public static String[] getServiceNames()
        {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param sServiceName Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService( String sServiceName )
        {
            return sServiceName.equals( __serviceName );
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public String getImplementationName()
        {
            return  _HelpSearch.class.getName();
        }

        //===================================================
        // XInvocation
        public XIntrospectionAccess getIntrospection()
        {
            return  null;
        }

        public Object invoke( String aFunctionName, java.lang.Object[] aParams,
            short[][] aOutParamIndex, java.lang.Object[][] aOutParam )
                throws  com.sun.star.lang.IllegalArgumentException,
                        com.sun.star.script.CannotConvertException,
                        com.sun.star.reflection.InvocationTargetException
        {
            String[] aRet = null;
            if( !aFunctionName.equals( aSearchMethodName ) )
                throw new com.sun.star.lang.IllegalArgumentException();

            Object[] aScoreOutArray = new Object[1];
            aScoreOutArray[0] = null;
            try
            {
                aRet =  doQuery( aParams, aScoreOutArray );
            }
            catch( Exception e )
            {
                aRet = null;
            }

            Object aScoreArray = aScoreOutArray[0];
            if( aScoreArray == null )
            {
                aOutParamIndex[0] = new short[0];
                aOutParam[0] = new Object[0];
            }
            else
            {
                short nInParamCount = (short)aParams.length;
                aOutParamIndex[0] = new short[1];
                aOutParamIndex[0][0] = nInParamCount;
                aOutParam[0] = new Object[1];
                aOutParam[0][0] = aScoreArray;
            }

            Any aRetAny = new Any( new Type( String[].class ), aRet );
            return aRetAny;
        }

        public void setValue( String aPropertyName, java.lang.Object aValue )
            throws  com.sun.star.beans.UnknownPropertyException,
                    com.sun.star.script.CannotConvertException,
                    com.sun.star.reflection.InvocationTargetException {
            throw new com.sun.star.beans.UnknownPropertyException();
        }

        public Object getValue( String aPropertyName )
            throws com.sun.star.beans.UnknownPropertyException {
            throw new com.sun.star.beans.UnknownPropertyException();
        }

        public boolean hasMethod( String aMethodName ) {
            boolean bRet = (aMethodName.equals( aSearchMethodName ) );
            return bRet;
        }
        public boolean hasProperty( String aName ) {
            return false;
        }

        // Command line interface for testing
        private static String[] doQuery( Object[] args, Object[] aScoreOutArray ) throws Exception
        {
             String aLanguageStr = "";
             String aIndexStr = "";
            String aQueryStr = "";
            boolean bCaptionOnly = false;

            int nParamCount = args.length;
            String aStrs[] = new String[nParamCount];
            for( int i = 0 ; i < nParamCount ; i++ )
            {
                try
                {
                    aStrs[i] = AnyConverter.toString( args[i] );
                }
                catch( IllegalArgumentException e )
                {
                    aStrs[i] = "";
                }
            }

            // TODO: Error handling
            for( int i = 0 ; i < nParamCount ; i++ )
            {
                if ("-lang".equals(aStrs[i]) )
                {
                    aLanguageStr = aStrs[i + 1];
                    i++;
                }
                else if( "-index".equals(aStrs[i]) )
                {
                    aIndexStr = aStrs[i+1];
                    i++;
                }
                else if( "-query".equals(aStrs[i]) )
                {
                    aQueryStr = aStrs[i+1];
                    i++;
                }
                else if( "-caption".equals(aStrs[i]) )
                {
                    bCaptionOnly = true;
                }
            }
            String[] aDocs = queryImpl( aLanguageStr, aIndexStr, aQueryStr, bCaptionOnly, aScoreOutArray );

            return aDocs;
        }

        private static String[] queryImpl( String aLanguageStr, String aIndexStr, String aQueryStr,
            boolean bCaptionOnly, Object[] aScoreOutArray ) throws Exception
        {
            File aIndexFile = new File( aIndexStr );
            IndexReader reader = IndexReader.open( NIOFSDirectory.open( aIndexFile ), true );
            Searcher searcher = new IndexSearcher( reader );
            Analyzer analyzer = aLanguageStr.equals("ja") ? (Analyzer)new CJKAnalyzer(Version.LUCENE_29) : (Analyzer)new StandardAnalyzer(Version.LUCENE_29);

            String aField;
            if( bCaptionOnly )
                aField = "caption";
            else
                aField = "content";

            Query aQuery;
            if( aQueryStr.endsWith( "*" ) )
                aQuery = new WildcardQuery( new Term( aField, aQueryStr ) );
            else
                aQuery = new TermQuery( new Term( aField, aQueryStr ) );

            // Perform search
            TopDocs aHits = searcher.search( aQuery, 100 );
            int nHitCount = aHits.scoreDocs.length;

            String aDocs[] = new String[nHitCount];
            float aScores[] = null;
            aScores = new float[nHitCount];
            for( int iHit = 0 ; iHit < nHitCount ; iHit++ )
            {
                ScoreDoc aDoc = aHits.scoreDocs[iHit];
                String aPath = searcher.doc(aDoc.doc).get( "path" );
                aDocs[iHit] = ( aPath != null ) ? aPath : "";
                aScores[iHit] = aDoc.score;
            }
            aScoreOutArray[0] = aScores;

            reader.close();

            return aDocs;
        }
    }

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( _HelpSearch.class.getName() ) )
            xFactory = Factory.createComponentFactory(_HelpSearch.class,
                                             _HelpSearch.getServiceNames());

        return xFactory;
    }

        /** This method is a member of the interface for initializing an object
         * directly after its creation.
         * @param object This array of arbitrary objects will be passed to the
         * component after its creation.
         * @throws Exception Every exception will not be handled, but will be
         * passed to the caller.
         */
        public void initialize( Object[] object )
            throws com.sun.star.uno.Exception {
            /* The component describes what arguments its expected and in which
             * order!At this point you can read the objects and can intialize
             * your component using these objects.
             */
        }
}

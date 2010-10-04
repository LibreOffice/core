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
import org.apache.lucene.search.Hits;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Searcher;
import org.apache.lucene.search.TermQuery;
import org.apache.lucene.search.WildcardQuery;

import com.sun.star.script.XInvocation;
import com.sun.star.beans.XIntrospectionAccess;

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
            IndexReader reader = IndexReader.open( aIndexStr );
            Searcher searcher = new IndexSearcher( reader );
            Analyzer analyzer = aLanguageStr.equals("ja") ? (Analyzer)new CJKAnalyzer() : (Analyzer)new StandardAnalyzer();

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
            Hits aHits = searcher.search( aQuery );
            int nHitCount = aHits.length();

            String aDocs[] = new String[nHitCount];
            float aScores[] = null;
            aScores = new float[nHitCount];
            for( int iHit = 0 ; iHit < nHitCount ; iHit++ )
            {
                Document aDoc = aHits.doc( iHit );
                String aPath = aDoc.get( "path" );
                aDocs[iHit] = ( aPath != null ) ? aPath : "";
                aScores[iHit] = aHits.score( iHit );
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

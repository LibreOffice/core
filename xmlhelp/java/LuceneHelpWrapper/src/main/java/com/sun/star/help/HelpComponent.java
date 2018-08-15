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
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.registry.XRegistryKey;

/*
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.FilterIndexReader;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.Term;
import org.apache.lucene.queryParser.QueryParser;
import org.apache.lucene.search.Hits;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.search.Searcher;
import org.apache.lucene.search.TermQuery;
import org.apache.lucene.search.WildcardQuery;

import com.sun.star.script.XInvocation;
import com.sun.star.beans.XIntrospectionAccess;

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.io.File;
*/

/** This class capsulates the class, that implements the minimal component and a
 * factory for creating the service (<CODE>__getComponentFactory</CODE>).
 */
public class HelpComponent
{
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

        if ( sImplName.equals( HelpSearch._HelpSearch.class.getName() ) )
            xFactory = Factory.createComponentFactory(HelpSearch._HelpSearch.class,
                                             HelpSearch._HelpSearch.getServiceNames());
        else if ( sImplName.equals( HelpIndexer.class.getName() ) )
            xFactory = Factory.createComponentFactory(HelpIndexer.class,
                                             HelpIndexer.getServiceNames());
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
        throws com.sun.star.uno.Exception
    {
        /* The component describes what arguments its expected and in which
         * order!At this point you can read the objects and can intialize
         * your component using these objects.
         */
    }
}

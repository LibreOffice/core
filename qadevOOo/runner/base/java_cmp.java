/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: java_cmp.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:03:16 $
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

package base;


import java.io.PrintWriter;

import stats.OutProducerFactory;
import lib.TestParameters;
import lib.TestCase;
import lib.Status;
import lib.MultiMethodTest;
import lib.TestEnvironment;
import util.DynamicClassLoader;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

import share.DescEntry;
import share.DescGetter;
import helper.APIDescGetter;
import helper.OfficeProvider;
import helper.AppProvider;
import base.TestBase;

import share.LogWriter;
import stats.Summarizer;

/**
 *
 * this class handles tests written in java without running an Office with
 * own bootstrapping of UNO
 */
public class java_cmp implements TestBase {

    private TestBase mWrappedTestBase = new java_fat();

    public boolean executeTest(lib.TestParameters param) {
        param.put("OfficeProvider", "helper.UnoProvider");
        return mWrappedTestBase.executeTest(param);
    }
}






































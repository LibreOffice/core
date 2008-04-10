/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: java_cmp.java,v $
 * $Revision: 1.4 $
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






































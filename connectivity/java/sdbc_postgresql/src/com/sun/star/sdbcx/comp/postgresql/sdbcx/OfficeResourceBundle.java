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

package com.sun.star.sdbcx.comp.postgresql.sdbcx;

import com.sun.star.lang.NullPointerException;
import com.sun.star.resource.MissingResourceException;
import com.sun.star.resource.OfficeResourceLoader;
import com.sun.star.resource.XResourceBundle;
import com.sun.star.resource.XResourceBundleLoader;
import com.sun.star.sdbcx.comp.postgresql.comphelper.CompHelper;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;

public class OfficeResourceBundle implements AutoCloseable {

    private XComponentContext context;
    private String baseName;
    private boolean haveAttemptedCreate;
    private XResourceBundle bundle;


    /** constructs a resource bundle
        @param  context
            the component context to operate in
        @param  bundleBaseName
            the base name of the resource file which should be accessed (*without* the SUPD!)
        @raises ::com::sun::star::lang::NullPointerException
            if the given component context is <NULL/>
     */
    public OfficeResourceBundle(XComponentContext context, String bundleBaseName) throws NullPointerException {
        if (context == null) {
            throw new NullPointerException();
        }
        this.context = context;
        this.baseName = bundleBaseName;
        this.haveAttemptedCreate = false;
    }

    @Override
    public void close() {
        CompHelper.disposeComponent(bundle);
    }

    /** loads the string with the given resource id from the resource bundle
        @param  _resourceId
            the id of the string to load
        @return
            the requested resource string. If no string with the given id exists in the resource bundle,
            an empty string is returned. In a non-product version, an OSL_ENSURE will notify you of this
            then.
    */
    public String loadString( int _resourceId ) {
        synchronized (this) {
            String ret = "";
            if (loadBundle()) {
                try {
                    Object value = bundle.getByName(getStringResourceKey(_resourceId));
                    ret = AnyConverter.toString(value);
                } catch (com.sun.star.uno.Exception ex) {
                }
            }
            return ret;
        }
    }

    /** determines whether the resource bundle has a string with the given id
        @param  _resourceId
            the id of the string whose existence is to be checked
        @return
            <TRUE/> if and only if a string with the given ID exists in the resource
            bundle.
    */
    public boolean hasString( int _resourceId ) {
        synchronized (this) {
            boolean ret = false;
            if (loadBundle()) {
                ret = bundle.hasByName(getStringResourceKey(_resourceId));
            }
            return ret;
        }
    }

    private String getStringResourceKey(int resourceId) {
        return "string:" + resourceId;
    }

    private boolean loadBundle() {
        if (haveAttemptedCreate) {
            return bundle != null;
        }
        haveAttemptedCreate = true;

        try {
            XResourceBundleLoader loader = OfficeResourceLoader.get(context);
            try {
                if (loader == null) {
                    return false;
                }
                try {
                    bundle = loader.loadBundle_Default(baseName);
                } catch (MissingResourceException missingResourceException) {
                }
                return bundle != null;
            } finally {
                CompHelper.disposeComponent(loader);
            }
        } catch (Exception exception) {
        }
        return false;
    }
}

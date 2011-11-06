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


package com.sun.star.wizards.web;

import com.sun.star.wizards.common.UCB;

/**
 * @author rpiterman
 * Verifies all String that do not end with
 * the given extension.
 * This is used to exclude from a copy all the
 * xsl files, so I copy from a layout directory
 * all the files that do *not* end with xsl.
 *
 */
public class ExtensionVerifier implements UCB.Verifier
{

    private String extension;

    public ExtensionVerifier(String extension_)
    {
        extension = "." + extension_;
    }

    /**
     * @return true if the given object is
     * a String which does not end with the
     * given extension.
     */
    public boolean verify(Object object)
    {
        if (object instanceof String)
        {
            return !((String) object).endsWith(extension);
        }
        return false;
    }
}

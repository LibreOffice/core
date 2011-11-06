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


/*
 * Created on 2005
 *  by Christian Schmidt
 */
package com.sun.star.tooling.converter;

import java.util.Map;

/**
 * Helps resolving restype descriptors used outside the tool to
 * restype descriptors used by this  tool
 *
 * @author Christian Schmidt 2005
 *
 */
public class ResTypeResolver {
    final static String[] inFields={"Text","QText","HText","Title"};
    final static String[] outFields={"res","res-quickhelp","res-help","res-title"};
    final static Map internKeys=new ExtMap(inFields,outFields);
    final static Map externKeys=new ExtMap(outFields,inFields);

    /**
     * Get the intern key depending to the given extern key
     *
     * @param externKey the externKey {"res","res-quickhelp","res-help","title"}
     * @return the depending intern key
     */
    public static String getInternKey(String externKey){
        return (String)externKeys.get(externKey);
    }

    /**
     * Get the extern key to the given intern key
     *
     * @param internKey the internal key
     * @return the external key
     */
    public static String getExternKey(String internKey){
        return (String)internKeys.get(internKey);
    }

}

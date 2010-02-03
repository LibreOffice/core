/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ResTypeResolver.java,v $
 * $Revision: 1.3 $
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

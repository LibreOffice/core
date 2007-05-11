/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResTypeResolver.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:10:04 $
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

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SxcPluginFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:30:58 $
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

package org.openoffice.xmerge.converter.xml.sxc;

import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.PluginFactory;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentMergerFactory;


/**
 *  General implementation of the <code>PluginFactory</code> interface
 *  for SXC <code>Document</code> objects.
 *
 *  @see  org.openoffice.xmerge.DocumentDeserializer
 *  @see  org.openoffice.xmerge.DocumentMerger
 *  @see  org.openoffice.xmerge.DocumentSerializer
 */
public abstract class SxcPluginFactory
    extends PluginFactory implements DocumentMergerFactory {


   /**
    *  Constructor that caches the <code>ConvertInfo</code> that
    *  corresponds to the registry information for this plug-in.
    *
    *  @param  ci  <code>ConvertInfo</code> object.
    */
    public SxcPluginFactory(ConverterInfo ci) {
        super(ci);
    }


    public Document createOfficeDocument(String name, InputStream is)
        throws IOException {

        // read zipped XML stream
        //
        SxcDocument doc = new SxcDocument(name);
        doc.read(is);
        return doc;
    }

     public Document createOfficeDocument(String name, InputStream is,boolean isZip)
        throws IOException {

        // read zipped XML stream
        //
        SxcDocument doc = new SxcDocument(name);
        doc.read(is,isZip);
        return doc;
    }
}


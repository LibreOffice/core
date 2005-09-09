/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Exporter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:03:34 $
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
 ************************************************************************/package com.sun.star.wizards.web.export;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.*;


/**
 *
 * @author  rpiterman
 */
public interface Exporter {
    /**
     * This method exports a document with a specified filter.<br/>
     * The method is responsible for exporting the given source document
     * to the given target directory.
     * The exporter *must* also set the given CGDocument sizeBytes field to the
     * size of the converted document, *if* the target document is
     * of a binary format.
     * always use source.urlFilename as destination filename.
     * @param source is a CGDocument object.
     * @param target contains the URL of a directory to which the
     * file should be exported to.
     * @param xmsf this is a basic multiServiceFactory.
     * @param task - a task monitoring object. This should advance
     * (call task.advacne(true) ) 3 times, while exporting.
     *
     */
    public boolean export(CGDocument source, String targetDirectory ,XMultiServiceFactory xmsf, Task task);

    public void init(CGExporter exporter);
}


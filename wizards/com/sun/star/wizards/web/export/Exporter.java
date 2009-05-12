/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Exporter.java,v $
 * $Revision: 1.6 $
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
package com.sun.star.wizards.web.export;

import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.*;

/**
 *
 * @author  rpiterman
 */
public interface Exporter
{

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
    public boolean export(CGDocument source, String targetDirectory, XMultiServiceFactory xmsf, Task task) throws IOException;

    public void init(CGExporter exporter);
}


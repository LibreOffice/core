/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.web.export;

import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.web.data.*;

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
     * @param targetDirectory contains the URL of a directory to which the file should be exported to.
     * @param xmsf this is a basic multiServiceFactory.
     * @param task - a task monitoring object. This should advance 
     * (call task.advance(true) ) 3 times, while exporting.
     * 
     */
    public boolean export(CGDocument source, String targetDirectory, XMultiServiceFactory xmsf, Task task) throws IOException;

    public void init(CGExporter exporter);
}


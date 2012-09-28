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

package graphical;

public interface IOffice
{
    /**
     * start an Office, if need
     * @throws graphical.OfficeException
     */
    public void start() throws OfficeException;

    /**
     * Load a document by it's Name
     * @param Name
     * @throws graphical.OfficeException
     */
    public void load(String Name) throws OfficeException;

    /**
     * Create a postscript file in the DOC_COMPARATOR_OUTPUT_DIR directory from a loaded document
     * @throws graphical.OfficeException
     */
    public void storeAsPostscript() throws OfficeException;


    /**
     * Close the background office
     * @throws graphical.OfficeException
     */
    public void close() throws OfficeException;
}

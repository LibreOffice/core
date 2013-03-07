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
package org.libreoffice.report;

import java.io.IOException;
import java.io.OutputStream;

/**
 * A repository for writing. Providing a repository always assumes,
 * that more than one stream can be written.
 *
 */
public interface OutputRepository
{

    /**
     * Creates an output stream for writing the data. If there is an entry with
     * that name already contained in the repository, try to overwrite it.
     *
     * @param name
     *    the name of the output stream
     * @param mimeType
     *    the mime type of the to-be-created output stream. Repository implementations which do not support
     *    associating a mime time with a stream might ignore this parameter.
     * @return the outputstream
     * @throws IOException if opening the stream fails
     */
    OutputStream createOutputStream(final String name, final String mimeType) throws IOException;

    /** allows to acces sub repositories inside this repository
     *
     * @param name describes the path to the sub repository
     * @param mimeType
     * @return the sub repository
     * @throws java.io.IOException when the sub repository doesn't exist.
     */
    OutputRepository openOutputRepository(final String name, final String mimeType) throws IOException;

    boolean exists(final String name);

    boolean existsStorage(final String name);

    boolean isWritable(final String name);

    void closeOutputRepository();
}

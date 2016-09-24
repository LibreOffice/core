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

/**
 * The {@code DiffAlgorithm} and {@code MergeAlgorithm} are used to provide
 * the merge capabilities of this project.
 *
 * <p>Merge is useful when an {@code OfficeDocument} is converted to a
 * {@literal "Device"} {@code Document} format, and the {@literal "Device"}
 * {@code Document} version is modified.
 * Those changes can be merged back into the original {@code OfficeDocument}
 * with the merger.  The merger is capable of doing this even if the
 * {@literal "Device"} format is lossy in comparison to the
 * {@code OfficeDocument} format.</p>
 *
 * <p>The {@code DiffAlgorithm} generates a list of {@code Difference} objects
 * that represent the differences between two {@code OfficeDocument} objects.
 * It is assumed that one is the original {@code OfficeDocument} object and the
 * other is a &quot;lossy&quot; version of the same {@code Document} with edits
 * to be merged.  Typically the {@literal "lossy"} version is created by
 * converting a {@literal "Device"} {@code Document} back into an
 * {@code OfficeDocument}.</p>
 *
 * <p>The {@code MergeAlgorithm} takes the {@code Difference} objects as input,
 * and creates a merged {@code OfficeDocument}.
 * A merged {@code OfficeDocument} has the following features:</p>
 *
 * <ul>
 * <li>Tags in the {@code OfficeDocument} that are not supported in the device
 *      format are not altered or removed.</li>
 * <li>Changes made to the device format are merged back into the
 *     {@code OfficeDocument} in the location determined by the
 *     {@code DiffAlgorithm}.</li>
 * </ul>
 *
 * <p>Each converter provides an implementation of the {@link
 * org.openoffice.xmerge.ConverterCapabilities ConverterCapabilities} which
 * specifies which {@code OfficeDocument} tags are supported for the device
 * format.</p>
 */
package org.openoffice.xmerge.merger;

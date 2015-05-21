/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

public class DocumentPartView {
    public final int partIndex;
    public final String partName;

    public DocumentPartView(int partIndex, String partName) {
        this.partIndex = partIndex;
        this.partName = partName;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.storage;

import org.libreoffice.storage.local.LocalDocumentsDirectoryProvider;
import org.libreoffice.storage.local.LocalDocumentsProvider;

/**
 * Keeps the instances of the available IDocumentProviders in the system.
 * Instances are maintained in a sorted list and providers have to be
 * accessed from their position.
 */
public class DocumentProviderFactory {

    private static IDocumentProvider[] providers = {
            new LocalDocumentsDirectoryProvider(), new LocalDocumentsProvider() };

    private static String[] providerNames = {
            "Local documents", "Local file system" };

    /**
     * Retrieve the provider associated to a certain position.
     *
     * @param position
     * @return document provider in that position.
     */
    public static IDocumentProvider getProvider(int position) {
        return providers[position];
    }

    /**
     * Returns a sorted list of the names of the providers. Order is meaningful
     * to retrieve the actual provider object with getProvider().
     *
     * @return Array with the names of the available providers.
     */
    public static String[] getNames() {
        return providerNames;
    }

    /**
     * Returns the default provider.
     *
     * @return default provider.
     */
    public static IDocumentProvider getDefaultProvider() {
        return providers[0];
    }
}

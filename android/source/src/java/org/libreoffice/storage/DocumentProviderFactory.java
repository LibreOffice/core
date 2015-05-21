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

import android.content.Context;

/**
 * Keeps the instances of the available IDocumentProviders in the system.
 * Instances are maintained in a sorted list and providers have to be
 * accessed from their position.
 *
 * The factory follows the Singleton pattern, there is only one instance of it
 * in the application and it must be retrieved with
 * DocumentProviderFactory.getInstance().
 */
public final class DocumentProviderFactory {

    /**
     * Private factory instance for the Singleton pattern.
     */
    private static DocumentProviderFactory instance = null;

    private IDocumentProvider[] providers = {
            new LocalDocumentsDirectoryProvider(), new LocalDocumentsProvider() };

    private String[] providerNames;

    private DocumentProviderFactory() {
        // private to prevent external instances of the factory
    }

    /**
     * Initializes the factory with some context. If this method is called for
     * twice or more times those calls will have no effect.
     *
     * @param context
     *            Application context for the factory.
     */
    public static void initialize(Context context) {
        if (instance == null) {
            // initialize instance
            instance = new DocumentProviderFactory();

            // initialize document providers list
            instance.providerNames = new String[instance.providers.length];
            for (int i = 0; i < instance.providers.length; i++) {
                instance.providerNames[i] = context.getString(instance
                        .getProvider(i).getNameResource());
            }
        }
    }

    /**
     * Retrieve the unique instance of the factory.
     *
     * @return the unique factory object or null if it is not yet initialized.
     */
    public static DocumentProviderFactory getInstance() {
        return instance;
    }

    /**
     * Retrieve the provider associated to a certain position.
     *
     * @param position
     * @return document provider in that position.
     */
    public IDocumentProvider getProvider(int position) {
        return providers[position];
    }

    /**
     * Returns a sorted list of the names of the providers. Order is meaningful
     * to retrieve the actual provider object with getProvider().
     *
     * @return Array with the names of the available providers.
     */
    public String[] getNames() {
        return providerNames;
    }

    /**
     * Returns the default provider.
     *
     * @return default provider.
     */
    public IDocumentProvider getDefaultProvider() {
        return providers[0];
    }
}

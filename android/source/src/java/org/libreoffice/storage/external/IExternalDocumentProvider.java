package org.libreoffice.storage.external;

import android.content.Context;

import org.libreoffice.storage.IDocumentProvider;


/**
 * Interface for external document providers.
 */
public interface IExternalDocumentProvider extends IDocumentProvider {

    /**
     * Used to obtain the default directory to display when
     * browsing using the internal DirectoryBrowser.
     *
     * @return a guess of the root file's URI.
     * @param context
     */
    String guessRootURI(Context context);

}

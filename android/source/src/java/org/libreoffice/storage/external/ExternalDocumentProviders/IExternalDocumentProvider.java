package org.libreoffice.storage.external.ExternalDocumentProviders;

import org.libreoffice.storage.IDocumentProvider;

public interface IExternalDocumentProvider extends IDocumentProvider {

    /**
     * Used to obtain the default directory to display when
     * browsing using the internal DirectoryBrowser.
     *
     * @return a guess of the root file's URI.
     */
    String guessRootURI();

}

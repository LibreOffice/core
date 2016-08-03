package org.libreoffice.storage.owncloud;

import java.io.File;
import java.io.FileFilter;
import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.libreoffice.storage.IFile;

import com.owncloud.android.lib.common.operations.RemoteOperationResult;
import com.owncloud.android.lib.resources.files.ChunkedUploadRemoteFileOperation;
import com.owncloud.android.lib.resources.files.DownloadRemoteFileOperation;
import com.owncloud.android.lib.resources.files.ReadRemoteFolderOperation;
import com.owncloud.android.lib.resources.files.RemoteFile;
import com.owncloud.android.lib.resources.files.UploadRemoteFileOperation;

/**
 * Implementation of IFile for ownCloud servers.
 */
public class OwnCloudFile implements IFile {

    private OwnCloudProvider provider;
    private RemoteFile file;

    private String name;
    private String parentPath;

    protected OwnCloudFile(OwnCloudProvider provider, RemoteFile file) {
        this.provider = provider;
        this.file = file;

        // get name and parent from path
        File localFile = new File(file.getRemotePath());
        this.name = localFile.getName();
        this.parentPath = localFile.getParent();
    }

    @Override
    public URI getUri(){

        try{
            return URI.create(URLEncoder.encode(file.getRemotePath(),"UTF-8"));
        }catch(UnsupportedEncodingException e){
            e.printStackTrace();
        }

        return null;
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public boolean isDirectory() {
        return file.getMimeType().equals("DIR");
    }

    @Override
    public long getSize() {
        return file.getLength();
    }

    @Override
    public Date getLastModified() {
        return new Date(file.getModifiedTimestamp());
    }

    @Override
    public List<IFile> listFiles() {
        List<IFile> children = new ArrayList<IFile>();
        if (isDirectory()) {
            ReadRemoteFolderOperation refreshOperation = new ReadRemoteFolderOperation(
                    file.getRemotePath());
            RemoteOperationResult result = refreshOperation.execute(provider
                    .getClient());
            if (!result.isSuccess()) {
                throw provider.buildRuntimeExceptionForResultCode(result.getCode());
            }
            for (Object obj : result.getData()) {
                RemoteFile child = (RemoteFile) obj;
                if (!child.getRemotePath().equals(file.getRemotePath()))
                    children.add(new OwnCloudFile(provider, child));
            }
        }
        return children;
    }

    @Override
    public List<IFile> listFiles(FileFilter filter) {
        List<IFile> children = new ArrayList<IFile>();
        if (isDirectory()) {
            ReadRemoteFolderOperation refreshOperation = new ReadRemoteFolderOperation(
                    file.getRemotePath());
            RemoteOperationResult result = refreshOperation.execute(provider
                    .getClient());
            if (!result.isSuccess()) {
                throw provider.buildRuntimeExceptionForResultCode(result.getCode());
            }

            for (Object obj : result.getData()) {
                RemoteFile child = (RemoteFile) obj;
                if (!child.getRemotePath().equals(file.getRemotePath())){
                    OwnCloudFile ownCloudFile = new OwnCloudFile(provider, child);
                    if(!ownCloudFile.isDirectory()){
                        File f = new File(provider.getCacheDir().getAbsolutePath(),
                                ownCloudFile.getName());
                        if(filter.accept(f))
                            children.add(ownCloudFile);
                        f.delete();
                    }else{
                        children.add(ownCloudFile);
                    }
                }
            }
        }
        return children;
    }

    @Override
    public IFile getParent() {
        if (parentPath == null)
            // this is the root node
            return null;

        return provider.createFromUri(URI.create(parentPath));
    }

    @Override
    public File getDocument() {
        if (isDirectory()) {
            return null;
        }
        File downFolder = provider.getCacheDir();
        DownloadRemoteFileOperation operation = new DownloadRemoteFileOperation(
                file.getRemotePath(), downFolder.getAbsolutePath());
        RemoteOperationResult result = operation.execute(provider.getClient());
        if (!result.isSuccess()) {
            throw provider.buildRuntimeExceptionForResultCode(result.getCode());
        }
        return new File(downFolder.getAbsolutePath() + file.getRemotePath());
    }

    @Override
    public boolean equals(Object object) {
        if (this == object)
            return true;
        if (!(object instanceof OwnCloudFile))
            return false;
        OwnCloudFile file = (OwnCloudFile) object;
        return file.getUri().equals(getUri());
    }

    @Override
    public void saveDocument(File newFile) {
        UploadRemoteFileOperation uploadOperation;
        if (newFile.length() > ChunkedUploadRemoteFileOperation.CHUNK_SIZE) {
            uploadOperation = new ChunkedUploadRemoteFileOperation(
                    newFile.getPath(), file.getRemotePath(), file.getMimeType());
        } else {
            uploadOperation = new UploadRemoteFileOperation(newFile.getPath(),
                    file.getRemotePath(), file.getMimeType());
        }

        RemoteOperationResult result = uploadOperation.execute(provider
                .getClient());
        if (!result.isSuccess()) {
            throw provider.buildRuntimeExceptionForResultCode(result.getCode());
        }
    }
}

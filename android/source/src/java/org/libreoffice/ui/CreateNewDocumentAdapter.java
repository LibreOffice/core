package org.libreoffice.ui;

import android.content.DialogInterface;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.RecyclerView;
import android.text.InputType;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;


import org.libreoffice.R;
import org.libreoffice.storage.IDocumentProvider;
import org.libreoffice.storage.IFile;
import org.libreoffice.storage.IOUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.List;

public class CreateNewDocumentAdapter extends RecyclerView.Adapter<CreateNewDocumentAdapter.ViewHolder> {

    private LibreOfficeUIActivity mActivity;
    private List<IFile> newFiles;

    private IDocumentProvider documentProvider;

    public CreateNewDocumentAdapter(LibreOfficeUIActivity mActivity, List<IFile> newFiles, IDocumentProvider documentProvider) {
        this.mActivity = mActivity;
        this.newFiles = newFiles;
        this.documentProvider = documentProvider;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View item = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.item_new_document, parent, false);
        return new ViewHolder(item);
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        final IFile iFile = newFiles.get(position);
        String filename = iFile.getName();

        holder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);
                builder.setTitle(R.string.dialog_filename);

                final EditText input = new EditText(mActivity);
                input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_CLASS_TEXT);
                builder.setView(input);
                DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        switch (which){
                            case DialogInterface.BUTTON_POSITIVE:
                                String newFilename = input.getText().toString();
                                String[] fileNameArray = iFile.getName().split("\\."); //getting the extension
                                newFilename = newFilename.trim() + "." +
                                        fileNameArray[fileNameArray.length-1]; //adding the extension to the file name
                                File file = saveToDocuments(iFile, newFilename);
                                try{
                                    mActivity.open(documentProvider.createFromUri(new URI(Uri.fromFile(file).toString())));
                                }catch (URISyntaxException e){
                                    e.printStackTrace();
                                }
                                break;
                            case DialogInterface.BUTTON_NEGATIVE:
                                dialog.cancel();
                                break;
                        }
                    }
                };
                builder.setPositiveButton(R.string.dialog_ok,dialogClickListener);
                builder.setNegativeButton(R.string.dialog_cancel,dialogClickListener);

                builder.show();

            }
        });

        int compoundDrawableInt = 0;

        switch (FileUtilities.getType(filename)) {
            case FileUtilities.DOC:
                compoundDrawableInt = R.drawable.writer;
                holder.textView.setText("Writer");
                break;
            case FileUtilities.CALC:
                compoundDrawableInt = R.drawable.calc;
                holder.textView.setText("Calc");
                break;
            case FileUtilities.DRAWING:
                compoundDrawableInt = R.drawable.draw;
                holder.textView.setText("Draw");
                break;
            case FileUtilities.IMPRESS:
                compoundDrawableInt = R.drawable.impress;
                holder.textView.setText("Impress");
                break;
        }
        holder.imageView.setImageDrawable(ContextCompat.getDrawable(mActivity, compoundDrawableInt));
    }

    @Override
    public int getItemCount() {
        return newFiles.size();
    }

    private File saveToDocuments(IFile file, String newFileName) {
        try{
            InputStream istream = mActivity.getAssets().open(file.getName());
            File storageFolder =new File(
                    Environment.getExternalStorageDirectory(), "Documents");
            File fileCopy = new File(storageFolder, newFileName);
            OutputStream ostream = new FileOutputStream(fileCopy);
            IOUtils.copy(istream, ostream);
            return fileCopy;

        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    class ViewHolder extends RecyclerView.ViewHolder {

        TextView textView;
        ImageView imageView;

        ViewHolder(View itemView) {
            super(itemView);
            this.textView = (TextView) itemView.findViewById(R.id.textView_new_document);
            this.imageView = (ImageView) itemView.findViewById(R.id.imageView_new_document);
        }
    }
}

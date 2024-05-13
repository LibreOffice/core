package org.libreoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import com.google.android.material.snackbar.Snackbar;
import androidx.core.content.FileProvider;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;

import org.json.JSONException;
import org.json.JSONObject;
import org.libreoffice.kit.Document;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import static org.libreoffice.SearchController.addProperty;

class FormattingController implements View.OnClickListener {
    private static final String LOGTAG = ToolbarController.class.getSimpleName();
    private static final int TAKE_PHOTO = 1;
    private static final int SELECT_PHOTO = 2;
    private static final int IMAGE_BUFFER_SIZE = 4 * 1024;

    private final LibreOfficeMainActivity mContext;
    private String mCurrentPhotoPath;

    FormattingController(LibreOfficeMainActivity context) {
        mContext = context;

        mContext.findViewById(R.id.button_insertFormatListBullets).setOnClickListener(this);
        mContext.findViewById(R.id.button_insertFormatListNumbering).setOnClickListener(this);
        mContext.findViewById(R.id.button_increaseIndent).setOnClickListener(this);
        mContext.findViewById(R.id.button_decreaseIndent).setOnClickListener(this);

        mContext.findViewById(R.id.button_bold).setOnClickListener(this);
        mContext.findViewById(R.id.button_italic).setOnClickListener(this);
        mContext.findViewById(R.id.button_strikethrough).setOnClickListener(this);
        mContext.findViewById(R.id.button_underlined).setOnClickListener(this);
        mContext.findViewById(R.id.button_clearformatting).setOnClickListener(this);

        mContext.findViewById(R.id.button_align_left).setOnClickListener(this);
        mContext.findViewById(R.id.button_align_center).setOnClickListener(this);
        mContext.findViewById(R.id.button_align_right).setOnClickListener(this);
        mContext.findViewById(R.id.button_align_justify).setOnClickListener(this);

        mContext.findViewById(R.id.button_insert_line).setOnClickListener(this);
        mContext.findViewById(R.id.button_insert_rect).setOnClickListener(this);
        mContext.findViewById(R.id.button_insert_picture).setOnClickListener(this);

        mContext.findViewById(R.id.button_insert_table).setOnClickListener(this);
        mContext.findViewById(R.id.button_delete_table).setOnClickListener(this);

        mContext.findViewById(R.id.button_font_shrink).setOnClickListener(this);
        mContext.findViewById(R.id.button_font_grow).setOnClickListener(this);

        mContext.findViewById(R.id.button_subscript).setOnClickListener(this);
        mContext.findViewById(R.id.button_superscript).setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        ImageButton button = (ImageButton) view;

        if (button.isSelected()) {
            button.getBackground().setState(new int[]{-android.R.attr.state_selected});
        } else {
            button.getBackground().setState(new int[]{android.R.attr.state_selected});
        }

        final int buttonId = button.getId();
        if (buttonId == R.id.button_insertFormatListBullets) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DefaultBullet"));
        } else if (buttonId == R.id.button_insertFormatListNumbering) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DefaultNumbering"));
        } else if (buttonId == R.id.button_increaseIndent) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:IncrementIndent"));
        } else if (buttonId == R.id.button_decreaseIndent) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DecrementIndent"));
        } else if (buttonId == R.id.button_bold) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Bold"));
        } else if (buttonId == R.id.button_italic) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Italic"));
        } else if (buttonId == R.id.button_strikethrough) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Strikeout"));
        } else if (buttonId == R.id.button_clearformatting) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:ResetAttributes"));
        } else if (buttonId == R.id.button_underlined) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:UnderlineDouble"));
        } else if (buttonId == R.id.button_align_left) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:LeftPara"));
        } else if (buttonId == R.id.button_align_center) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:CenterPara"));
        } else if (buttonId == R.id.button_align_right) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:RightPara"));
        } else if (buttonId == R.id.button_align_justify) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:JustifyPara"));
        } else if (buttonId == R.id.button_insert_line) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Line"));
        } else if (buttonId == R.id.button_insert_rect) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Rect"));
        } else if (buttonId == R.id.button_font_shrink) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Shrink"));
        } else if (buttonId == R.id.button_font_grow) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Grow"));
        } else if (buttonId == R.id.button_subscript) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:SubScript"));
        }else if (buttonId == R.id.button_superscript) {
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:SuperScript"));
        } else if (buttonId == R.id.button_insert_picture) {
            insertPicture();
        } else if (buttonId == R.id.button_insert_table) {
            insertTable();
        } else if (buttonId == R.id.button_delete_table) {
            deleteTable();
        }
    }

    void onToggleStateChanged(final int type, final boolean selected) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                Integer buttonId;
                switch (type) {
                    case Document.BOLD:
                        buttonId = R.id.button_bold;
                        break;
                    case Document.ITALIC:
                        buttonId = R.id.button_italic;
                        break;
                    case Document.UNDERLINE:
                        buttonId = R.id.button_underlined;
                        break;
                    case Document.STRIKEOUT:
                        buttonId = R.id.button_strikethrough;
                        break;
                    case Document.ALIGN_LEFT:
                        buttonId = R.id.button_align_left;
                        break;
                    case Document.ALIGN_CENTER:
                        buttonId = R.id.button_align_center;
                        break;
                    case Document.ALIGN_RIGHT:
                        buttonId = R.id.button_align_right;
                        break;
                    case Document.ALIGN_JUSTIFY:
                        buttonId = R.id.button_align_justify;
                        break;
                    case Document.BULLET_LIST:
                        buttonId = R.id.button_insertFormatListBullets;
                        break;
                    case Document.NUMBERED_LIST:
                        buttonId = R.id.button_insertFormatListNumbering;
                        break;
                    default:
                        Log.e(LOGTAG, "Uncaptured state change type: " + type);
                        return;
                }

                ImageButton button = mContext.findViewById(buttonId);
                button.setSelected(selected);
                if (selected) {
                    button.getBackground().setState(new int[]{android.R.attr.state_selected});
                } else {
                    button.getBackground().setState(new int[]{-android.R.attr.state_selected});
                }
            }
        });
    }

    private void insertPicture() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        String[] options = {mContext.getResources().getString(R.string.take_photo),
                mContext.getResources().getString(R.string.select_photo)};
        builder.setItems(options, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case 0:
                        dispatchTakePictureIntent();
                        break;
                    case 1:
                        sendImagePickingIntent();
                        break;
                    default:
                        sendImagePickingIntent();
                }
            }
        });
        builder.show();
    }

    private void insertTable() {
        final AlertDialog.Builder insertTableBuilder = new AlertDialog.Builder(mContext);
        insertTableBuilder.setTitle(R.string.insert_table);
        LayoutInflater layoutInflater = mContext.getLayoutInflater();
        View numberPicker = layoutInflater.inflate(R.layout.number_picker, null);
        final int minValue = 1;
        final int maxValue = 20;
        TextView npRowPositive = numberPicker.findViewById(R.id.number_picker_rows_positive);
        TextView npRowNegative = numberPicker.findViewById(R.id.number_picker_rows_negative);
        TextView npColPositive = numberPicker.findViewById(R.id.number_picker_cols_positive);
        TextView npColNegative = numberPicker.findViewById(R.id.number_picker_cols_negative);
        final TextView npRowCount = numberPicker.findViewById(R.id.number_picker_row_count);
        final TextView npColCount = numberPicker.findViewById(R.id.number_picker_col_count);

        View.OnClickListener positiveButtonClickListener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int rowCount = Integer.parseInt(npRowCount.getText().toString());
                int colCount = Integer.parseInt(npColCount.getText().toString());
                final int id = v.getId();
                if (id == R.id.number_picker_rows_positive && rowCount < maxValue) {
                    npRowCount.setText(String.valueOf(++rowCount));
                } else if (id == R.id.number_picker_cols_positive && colCount < maxValue) {
                    npColCount.setText(String.valueOf(++colCount));
                }
            }
        };

        View.OnClickListener negativeButtonClickListener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int rowCount = Integer.parseInt(npRowCount.getText().toString());
                int colCount = Integer.parseInt(npColCount.getText().toString());
                final int id = v.getId();
                if (id == R.id.number_picker_rows_negative && rowCount > minValue) {
                    npRowCount.setText(String.valueOf(--rowCount));
                } else if (id == R.id.number_picker_cols_negative && colCount > minValue) {
                    npColCount.setText(String.valueOf(--colCount));
                }
            }
        };

        npRowPositive.setOnClickListener(positiveButtonClickListener);
        npColPositive.setOnClickListener(positiveButtonClickListener);
        npRowNegative.setOnClickListener(negativeButtonClickListener);
        npColNegative.setOnClickListener(negativeButtonClickListener);

        insertTableBuilder.setView(numberPicker);
        insertTableBuilder.setNeutralButton(R.string.alert_cancel, null);
        insertTableBuilder.setPositiveButton(R.string.alert_ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

                try {
                    JSONObject cols = new JSONObject();
                    cols.put("type", "long");
                    cols.put("value", Integer.valueOf(npColCount.getText().toString()));
                    JSONObject rows = new JSONObject();
                    rows.put("type","long");
                    rows.put("value",Integer.valueOf(npRowCount.getText().toString()));
                    JSONObject params = new JSONObject();
                    params.put("Columns", cols);
                    params.put("Rows", rows);
                    LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:InsertTable",params.toString()));
                    LibreOfficeMainActivity.setDocumentChanged(true);
                } catch (JSONException e) {
                    e.printStackTrace();
                }

            }
        });

        AlertDialog.Builder insertBuilder = new AlertDialog.Builder(mContext);
        insertBuilder.setTitle(R.string.select_insert_options);
        insertBuilder.setNeutralButton(R.string.alert_cancel, null);
        final int[] selectedItem = new int[1];
        insertBuilder.setSingleChoiceItems(mContext.getResources().getStringArray(R.array.insertrowscolumns), -1, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                selectedItem[0] = which;
            }
        });
        insertBuilder.setPositiveButton(R.string.alert_ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (selectedItem[0]){
                    case 0:
                        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:InsertRowsBefore"));
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        break;
                    case 1:
                        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:InsertRowsAfter"));
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        break;
                    case 2:
                        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:InsertColumnsBefore"));
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        break;
                    case 3:
                        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:InsertColumnsAfter"));
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        break;
                    case 4:
                        insertTableBuilder.show();
                        break;

                }
            }
        });
        insertBuilder.show();
    }

    private void deleteTable() {
        AlertDialog.Builder deleteBuilder = new AlertDialog.Builder(mContext);
        deleteBuilder.setTitle(R.string.select_delete_options);
        deleteBuilder.setNeutralButton(R.string.alert_cancel,null);
        final int[] selectedItem = new int[1];
        deleteBuilder.setSingleChoiceItems(mContext.getResources().getStringArray(R.array.deleterowcolumns), -1, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                selectedItem[0] = which;
            }
        });
        deleteBuilder.setPositiveButton(R.string.alert_ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (selectedItem[0]){
                    case 0:
                        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DeleteRows"));
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        break;
                    case 1:
                        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DeleteColumns"));
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        break;
                    case 2:
                        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DeleteTable"));
                        LibreOfficeMainActivity.setDocumentChanged(true);
                        break;
                }
            }
        });
        deleteBuilder.show();
    }

    private void sendImagePickingIntent() {
        Intent intent = new Intent(Intent.ACTION_PICK);
        intent.setType("image/*");
        mContext.startActivityForResult(Intent.createChooser(intent,
                mContext.getResources().getString(R.string.select_photo_title)), SELECT_PHOTO);
    }

    private void dispatchTakePictureIntent() {
        if (!mContext.getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_ANY)) {
            Snackbar.make(mContext.findViewById(R.id.button_insert_picture),
                    mContext.getResources().getString(R.string.no_camera_found), Snackbar.LENGTH_SHORT).show();
            return;
        }
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        // Create the File where the photo should go
        File photoFile = null;
        try {
            photoFile = createImageFile();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        // Continue only if the File was successfully created
        if (photoFile != null) {
            Uri photoURI = FileProvider.getUriForFile(mContext,
                    mContext.getPackageName() + ".fileprovider", photoFile);
            takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoURI);
            mContext.startActivityForResult(takePictureIntent, TAKE_PHOTO);
        }
    }

    void handleActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == TAKE_PHOTO && resultCode == Activity.RESULT_OK) {
            compressAndInsertImage();
        } else if (requestCode == SELECT_PHOTO && resultCode == Activity.RESULT_OK) {
            getFileFromURI(data.getData());
            compressAndInsertImage();
        }
    }

    void compressAndInsertImage() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        String[] options = {mContext.getResources().getString(R.string.compress_photo_smallest_size),
                mContext.getResources().getString(R.string.compress_photo_medium_size),
                mContext.getResources().getString(R.string.compress_photo_max_quality),
                mContext.getResources().getString(R.string.compress_photo_no_compress)};
        builder.setTitle(mContext.getResources().getString(R.string.compress_photo_title));
        builder.setItems(options, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                int compressGrade;
                switch (which) {
                    case 0:
                        compressGrade = 0;
                        break;
                    case 1:
                        compressGrade = 50;
                        break;
                    case 2:
                        compressGrade = 100;
                        break;
                    case 3:
                        compressGrade = -1;
                        break;
                    default:
                        compressGrade = -1;
                }
                compressImage(compressGrade);
                sendInsertGraphic();
            }
        });
        builder.show();
    }

    private void getFileFromURI(Uri uri) {
        try {
            InputStream input = mContext.getContentResolver().openInputStream(uri);
            mCurrentPhotoPath = createImageFile().getAbsolutePath();
            FileOutputStream output = new FileOutputStream(mCurrentPhotoPath);
            if (input != null) {
                byte[] buffer = new byte[IMAGE_BUFFER_SIZE];
                int read;
                while ((read = input.read(buffer)) != -1) {
                    output.write(buffer, 0, read);
                }
                input.close();
            }
            output.flush();
            output.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void sendInsertGraphic() {
        JSONObject rootJson = new JSONObject();
        try {
            addProperty(rootJson, "FileName", "string", "file://" + mCurrentPhotoPath);
        } catch (JSONException ex) {
            ex.printStackTrace();
        }
        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:InsertGraphic", rootJson.toString()));
        LOKitShell.sendEvent(new LOEvent(LOEvent.REFRESH));
        mContext.setDocumentChanged(true);
    }

    private void compressImage(int grade) {
        if (grade < 0 || grade > 100) {
            return;
        }
        mContext.showProgressSpinner();
        Bitmap bmp = BitmapFactory.decodeFile(mCurrentPhotoPath);
        try {
            mCurrentPhotoPath = createImageFile().getAbsolutePath();
            FileOutputStream out = new FileOutputStream(mCurrentPhotoPath);
            bmp.compress(Bitmap.CompressFormat.JPEG, grade, out);
        } catch (Exception e) {
            e.printStackTrace();
        }
        mContext.hideProgressSpinner();
    }

    private File createImageFile() throws IOException {
        // Create an image file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = mContext.getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        File image = File.createTempFile(
                imageFileName,  /* prefix */
                ".jpg",         /* suffix */
                storageDir      /* directory */
        );
        // Save a file: path for use with ACTION_VIEW intents
        mCurrentPhotoPath = image.getAbsolutePath();
        return image;
    }
}

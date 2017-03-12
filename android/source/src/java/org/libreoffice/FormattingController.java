package org.libreoffice;

import android.util.Log;
import android.view.View;
import android.widget.ImageButton;

import org.libreoffice.kit.Document;

 class FormattingController implements View.OnClickListener {
    private static final String LOGTAG = ToolbarController.class.getSimpleName();

    private LibreOfficeMainActivity mContext;

    FormattingController(LibreOfficeMainActivity context) {
        mContext = context;

        mContext.findViewById(R.id.button_bold).setOnClickListener(this);
        mContext.findViewById(R.id.button_italic).setOnClickListener(this);
        mContext.findViewById(R.id.button_strikethrough).setOnClickListener(this);
        mContext.findViewById(R.id.button_underlined).setOnClickListener(this);

        mContext.findViewById(R.id.button_align_left).setOnClickListener(this);
        mContext.findViewById(R.id.button_align_center).setOnClickListener(this);
        mContext.findViewById(R.id.button_align_right).setOnClickListener(this);
        mContext.findViewById(R.id.button_align_justify).setOnClickListener(this);
        mContext.findViewById(R.id.button_numbered_list).setOnClickListener(this);
        mContext.findViewById(R.id.button_bullet_list).setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        ImageButton button = (ImageButton) view;

        if (button.isSelected()) {
            button.getBackground().setState(new int[]{-android.R.attr.state_selected});
        } else {
            button.getBackground().setState(new int[]{android.R.attr.state_selected});
        }

        switch(button.getId()) {
            case R.id.button_bold:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Bold"));
                break;
            case R.id.button_italic:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Italic"));
                break;
            case R.id.button_strikethrough:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Strikeout"));
                break;
            case R.id.button_underlined:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:Underline"));
                break;
            case R.id.button_align_left:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:LeftPara"));
                break;
            case R.id.button_align_center:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:CenterPara"));
                break;
            case R.id.button_align_right:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:RightPara"));
                break;
            case R.id.button_align_justify:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:JustifyPara"));
                break;
            case R.id.button_bullet_list:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DefaultBullet"));
                break;
            case R.id.button_numbered_list:
                LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:DefaultNumbering"));
                break;
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
                        buttonId = R.id.button_bullet_list;
                        break;
                    case Document.NUMBERED_LIST:
                        buttonId = R.id.button_numbered_list;
                        break;
                    default:
                        Log.e(LOGTAG, "Uncaptured state change type: " + type);
                        return;
                }

                ImageButton button = (ImageButton) mContext.findViewById(buttonId);
                button.setSelected(selected);
                if (selected) {
                    button.getBackground().setState(new int[]{android.R.attr.state_selected});
                } else {
                    button.getBackground().setState(new int[]{-android.R.attr.state_selected});
                }
            }
        });
    }
}

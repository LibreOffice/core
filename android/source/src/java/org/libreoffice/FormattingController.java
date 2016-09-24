package org.libreoffice;

import android.content.Context;
import android.support.v7.app.ActionBar;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.ImageButton;

import org.libreoffice.kit.Document;

public class FormattingController implements View.OnClickListener {
    private static final String LOGTAG = ToolbarController.class.getSimpleName();

    private final Toolbar mToolbarBottom;
    private LibreOfficeMainActivity mContext;

    public FormattingController(LibreOfficeMainActivity context, Toolbar toolbarBottom) {
        mToolbarBottom = toolbarBottom;
        mContext = context;

        ((ImageButton) context.findViewById(R.id.button_bold)).setOnClickListener(this);
        ((ImageButton) context.findViewById(R.id.button_italic)).setOnClickListener(this);
        ((ImageButton) context.findViewById(R.id.button_strikethrough)).setOnClickListener(this);
        ((ImageButton) context.findViewById(R.id.button_underlined)).setOnClickListener(this);

        ((ImageButton) context.findViewById(R.id.button_align_left)).setOnClickListener(this);
        ((ImageButton) context.findViewById(R.id.button_align_center)).setOnClickListener(this);
        ((ImageButton) context.findViewById(R.id.button_align_right)).setOnClickListener(this);
        ((ImageButton) context.findViewById(R.id.button_align_justify)).setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        ImageButton button = (ImageButton) view;
        boolean selected = button.isSelected();
        button.setSelected(selected);

        if (selected) {
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
            default:
                break;
        }
    }

    public void onToggleStateChanged(final int type, final boolean selected) {
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                Integer buttonId = null;
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
                    default:
                        Log.e(LOGTAG, "Uncaptured state change type: " + type);
                        return;
                }

                LibreOfficeMainActivity activity = LibreOfficeMainActivity.mAppContext;
                ImageButton button = (ImageButton) activity.findViewById(buttonId);
                button.setSelected(selected);
                if (selected) {
                    button.getBackground().setState(new int[]{android.R.attr.state_selected});
                } else {
                    button.getBackground().setState(new int[]{-android.R.attr.state_selected});
                }
            }
        });


        /*if (menuItem == null) {
            Log.e(LOGTAG, "MenuItem not found.");
            return;
        }

        final Drawable drawable;
        if (pressed) {
            Resources resources = mContext.getResources();
            Drawable[] layers = new Drawable[2];
            layers[0] = resources.getDrawable(R.drawable.icon_background);
            layers[1] = resources.getDrawable(drawableId);
            drawable = new LayerDrawable(layers);
        } else {
            drawable = mContext.getResources().getDrawable(drawableId);
        }

        final MenuItem fMenuItem = menuItem;
        LOKitShell.getMainHandler().post(new Runnable() {
            public void run() {
                fMenuItem.setIcon(drawable);
            }
        });*/
    }
}

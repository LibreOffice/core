//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit



class DocumentController: UIViewController, MenuDelegate, UIDocumentPickerDelegate
{
    var currentDocumentName : String?
    var currentCloudUrl : URL?


    public func documentPicker(_ controller: UIDocumentPickerViewController, didPickDocumentAt url: URL)
    {
        currentCloudUrl = url
        currentDocumentName = url.lastPathComponent
    }

    func documentPickerWasCancelled(_ controller: UIDocumentPickerViewController)
    {
        currentCloudUrl = nil
        currentDocumentName = nil
    }


    @IBOutlet weak var janTest: UILabel!


    // Show sidemenu (part of documentcontroller)
    @IBAction func doMenu(_ sender: UIBarButtonItem)
    {
        if (sender.tag == 10) {
            sender.tag = 0;

            let viewMenuBack : UIView = view.subviews.last!

            UIView.animate(withDuration: 0.3, animations: { () -> Void in
                var frameMenu : CGRect = viewMenuBack.frame
                frameMenu.origin.x = -1 * UIScreen.main.bounds.size.width
                viewMenuBack.frame = frameMenu
                viewMenuBack.layoutIfNeeded()
                viewMenuBack.backgroundColor = UIColor.clear
                }, completion: { (finished) -> Void in
                    viewMenuBack.removeFromSuperview()
                })
            return
        }

        sender.isEnabled = false
        sender.tag = 10

        let sidebar : SidebarController = self.storyboard!.instantiateViewController(withIdentifier: "SidebarController") as! SidebarController
        view.addSubview(sidebar.view)
        addChildViewController(sidebar)
        sidebar.view.layoutIfNeeded()


        sidebar.view.frame=CGRect(x: 0 - UIScreen.main.bounds.size.width, y: 0, width: UIScreen.main.bounds.size.width, height: UIScreen.main.bounds.size.height);

        UIView.animate(withDuration: 0.3, animations: { () -> Void in
            sidebar.view.frame=CGRect(x: 0, y: 0, width: UIScreen.main.bounds.size.width, height: UIScreen.main.bounds.size.height);
            sender.isEnabled = true
            }, completion:nil)
    }



    // Last stop before displaying popover
    override func prepare(for segue: UIStoryboardSegue, sender: Any?)
    {
        if segue.identifier == "showActions" {
            let vc = segue.destination as! DocumentActions
            vc.delegate = self

            // JIX, TO BE CHANGED
            vc.isDocActive = true
        }
    }




    func actionName(_ name : String)
    {
        currentDocumentName = name
    }



    func actionMenuSelected(_ tag : Int)
    {
        switch tag
        {
            case 1: // New
                print("menu New to be done")

            case 2: // Open...
                let openMenu = UIDocumentPickerViewController(documentTypes: ["public.content"], in: .open)
                openMenu.delegate = self
                self.present(openMenu, animated: true, completion: nil)
                print("menu Open... to be done")

            case 3: // Save
                print("menu Save to be done")

            case 4: // Save as...
                let vc = storyboard?.instantiateViewController(withIdentifier: "setNameAction") as! setNameAction
                vc.modalPresentationStyle = .popover
                vc.delegate = self
                let popover = vc.popoverPresentationController!
                popover.delegate = self as? UIPopoverPresentationControllerDelegate
                popover.permittedArrowDirections = .up
                popover.sourceView = janTest
                popover.sourceRect = janTest.bounds
                present(vc, animated: true, completion: nil)

                print("menu Save as... to be done")
            case 5: // Save as PDF...
                print("menu Save as PDF... to be done")

            case 6: // Print...
                print("menu Print... to be done")

            default: // should not happen
                print("unknown menu" + String(tag))
        }
    }


    override func viewDidLoad()
    {
        super.viewDidLoad()
        // Do any additional setup after loading the view.

        // let path = Bundle.main.path(forResource: "Info", ofType: "plist")
        // let dict = NSDictionary(contentsOfFile: path!)
        // let tableData = dict!.object(forKey: "CFBundleDocumentTypes")
    }



    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}



// Protocol for action popover callback
protocol MenuDelegate
{
    func actionMenuSelected(_ tag : Int)
    func actionName(_ name : String)
}



class DocumentActions: UITableViewController
{
    // Pointer to callback class
    var delegate  : MenuDelegate?
    var isDocActive : Bool = false

    // Calling class might enable/disable each button
    @IBOutlet weak var buttonNew: UIButton!
    @IBOutlet weak var buttonOpen: UIButton!
    @IBOutlet weak var buttonSave: UIButton!
    @IBOutlet weak var buttonSaveAs: UIButton!
    @IBOutlet weak var buttonSaveAsPDF: UIButton!
    @IBOutlet weak var buttonPrint: UIButton!



    @IBAction func actionMenuSelect(_ sender: UIButton)
    {
        dismiss(animated: false)
        delegate?.actionMenuSelected(sender.tag)
    }


    override func viewDidLoad()
    {
        super.viewDidLoad()
        buttonSave.isEnabled = isDocActive
        buttonSaveAs.isEnabled = isDocActive
        buttonSaveAsPDF.isEnabled = isDocActive
        buttonPrint.isEnabled = isDocActive
    }
}



class setNameAction: UIViewController
{
    // Pointer to callback class
    var delegate  : MenuDelegate?
    var didEdit : Bool = false

    // reference to new name
    @IBOutlet weak var editText: UITextField!


    // continue "save as..." with new name
    @IBAction func actionOK(_ sender: UIButton)
    {
        dismiss(animated: false)
        if didEdit && editText.text != "" {
            delegate?.actionName(editText.text!)
        }
    }


    @IBAction func actionStartEdit(_ sender: UITextField)
    {
        if !didEdit {
            sender.text = ""
            didEdit = true
        }
    }

    @IBAction func actionCancel(_ sender: UIButton)
    {
        dismiss(animated: false)
    }


    override func viewDidLoad()
    {
        super.viewDidLoad()
    }
}


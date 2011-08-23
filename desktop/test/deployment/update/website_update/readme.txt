The folder contains extensions which need to be updated through a web site.
The "Updates dialog" of the Extension Manager will mark the updates for these
extensions as "browser based update". The Extension Manager will open a browser 
for each of the extensions and navigate to the respective website. 

================================================================================
web1.oxt - web5.oxt: 
================================================================================
They contain <update-information>. That is they reference directly the respective
webX.update.xml (for example, web1.update.xml) files which are available at
http://extensions.openoffice.org/testarea/desktop/website_update/update/...
For example:
http://extensions.openoffice.org/testarea/desktop/website_update/update/web2.update.xml

The update information contain multiple URLs to "localized" web sites. Each URL is 
assigned to a particular local. For example:

<src xlink:href="http://extensions.openoffice.org/testarea/desktop/website_update/update/web1_de.html" lang="de"/>

The Extension Manager will choose the URLs where the lang attribute matches most 
closely the locale of the office.

The following table shows what localized web site  is used, when the office uses the locale 
en-US. The web page will display the locale used. See update/web1_de.html, etc.


Localization:

Installed office en-US
         | publisher     | release notes
=============================================
web1.oxt | en-US         | en-US
--------------------------------------------- 
web2.oxt | en-US-region1 | en-US-region1
---------------------------------------------
web3.oxt | en            | en
---------------------------------------------
web4.oxt | en-GB         | en-GB
---------------------------------------------
web5.oxt | de            | de


================================================================================
web6.oxt
================================================================================
Need not be committed in extensions/www/testarea/desktop.

web6.oxt is intended for tests with extensions.services.openoffice.org.
It does not contain <update-information>. That is, the Extension Manager will obtain
the update information from the repository as feed:

http://updateext.services.openoffice.org/ProductUpdateService/check.Update

Test
----
Repository:

-Create the new extension in the repository. 
-Provide a company name and a URL to the company website. In our case this should be
 http://extensions.openoffice.org/testarea/desktop/website_update/update/web1_en-US.html
  
-Create a new release for the extension. Upload the description.xml of version 2.0 
(update/web6/description.xml). Provide a download URL for the web site (field 
 "Download from page / Open follow up page URL", which should be 
  http://extensions.openoffice.org/testarea/desktop/website_update/update/web1_en-US.html
  Provide release notes.
 

Office:
-Install version 1.0 of the extension:
 desktop/test/deployment/update/website_update/web6.oxt

-Run the update in the Extension Manager


Result:
The Update Dialog should show the publisher name as provided in "Provider URL" field 
of the extension edit page (not release).

A release notes link is displayed with an URL to the release notes as provided in 
the "Provider Title" field of the extension release edit page.

When running the update then the web browser should navigate to
http://extensions.openoffice.org/testarea/desktop/website_update/update/web1_en-US.html


================================================================================
web7.oxt
================================================================================
Need not be committed in extensions/www/testarea/desktop

web7.oxt is intended for tests with extensions.services.openoffice.org.
It does not contain <update-information>. That is, the Extension Manager will obtain
the update information from the repository as feed:

http://updateext.services.openoffice.org/ProductUpdateService/check.Update

The description.xml which will be uploaded contains URLs for release notes and publisher 
names/ URLs. That is, these information are not generated from the information of the
repository web site.

Test
-----------
Repository:

-Create the new extension in the repository. 
-Provide a company name and a URL to the company website. In our case these should be different
 to those provided in the description.xml. These should NOT go into the update feed.
 Choose for example as "Provider Title": FOO and as "Provider URL" some valid URL but NOT
 http://extensions.openoffice.org/testarea/desktop/website_update/update/web1_en-US.html

-Create a new release for the extension. Upload the description.xml of version 2.0 
(update/web7/description.xml). Provide a download URL for the web site (field 
 "Download from page / Open follow up page URL", which should be 
  http://extensions.openoffice.org/testarea/desktop/website_update/update/web1_en-US.html
  Provide release notes.

Office:
-Install version 1.0 of the extension:
 desktop/test/deployment/update/website_update/web7.oxt

-Run the update in the Extension Manager

Result:
The Update Dialog should show the publisher name as provided in the description.xml.
That is: My OpenOffice en-US and NOT "FOO".

A release notes link is displayed with an URL to the release notes as provided in 
the description.xml. That is:
http://extensions.openoffice.org/testarea/desktop/publisher/release-notes_xxx.html 

When running the update then the web browser should navigate to
http://extensions.openoffice.org/testarea/desktop/website_update/update/web1_en-US.html

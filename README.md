# LibreOffice - MIMO version


<img src="mimo+linagora.png" alt="mimo+linagora" width="600"/>

This is the [MIMO](https://www.journal-officiel.gouv.fr/mimo/) version of LibreOffice, developed by [LINAGORA](https://linagora.com/).

## Download

We make Windows builds in 32 and 64 bits.

The most recent version reviewed by MIMO can be downloaded at <https://www.mim.ovh/mimo/>


## Release notes

### `6.1.6.3.M13`

* Fix [acim#1418](https://acim.08000linux.com/issues/1418): Nouvelles failles dans libreoffice by fixing two CVE:
	- CVE-2019-9855
	- CVE-2019-9854


### `6.1.6.3.M12`

* Fix [acim#1408](https://acim.08000linux.com/issues/1404): Demande d'intégration du patch de sécurité pour LibreOffice by fixing two CVE:
	- CVE-2019-9848
	- CVE-2019-9849

### `6.1.6.3.M11`

* Revert `tosca#24996: Recolor the shadow in the renderer`: the patch generated regressions
* Fix [acim#1404: Publipostage et Barre d'outils Mailing (problématique numérotation)](https://acim.08000linux.com/issues/1404) by reverting:
	- `tdf#121606: displaying Mail Merge toolbar shouldn't activate connection`
	- `tdf#121607 defer other requests until password dialog is dismissed`

### `6.1.6.3.M10`

* `acim#1271: Fix publipostage` replaced by a fix from upstream: `tdf#122198 don't remove db after close`
* `tosca#24347: Pb ombrage dans impress` replaced by `tosca#24996: Recolor the shadow in the renderer`

### `6.1.6.3.M9`

MIMO doesn't yet want to switch to LibreOffice `6.2.x`. They want the patches from Fiducial in a `6.1.x` version, excluding the patch for acim#1133 #1290 and #1311: <https://acim.08000linux.com/issues/1311#note-19>. Therefore we can't ship the same version to MIMO and to the other clients.

**Patches from `6.1.5.2.M8` (MIMO):**

* `acim#1026: Fix with commit af11abf3`
	- Cherry-picked
* `acim#1271: Fix publipostage`
	- Cherry-picked
* `acim#910: Fix text color when printing`
	- Cherry-picked
* `ACIM1319: Clean SQL string`
	- Cherry-picked

**Patches from `6.2.3.2` (Fiducial):**

* `tdf#124824 FILEOPEN: Error rendering page break between two tables in a RTF`
	- Cherry-picked
* `tosca#24347: Fix shadow impress` and `tosca#24347: Fix shadow impress 2/2`
	- Cherry-picked
	- Squashed into `tosca#24347: Pb ombrage dans impress`
* `tosca#24253: souci décalage RTF sous Writer`
	- Cherry-picked
* Patches related to acim#1133 #1290 and #1311
	- Not cherry-picked

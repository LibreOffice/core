// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Definitions.Menu - JSON description of menus for JSDialog
 */

declare var JSDialog: any;

const menuDefinitions = new Map<string, Array<MenuDefinition>>();
enum Paper {
	PAPER_A0,
	PAPER_A1,
	PAPER_A2,
	PAPER_A3,
	PAPER_A4,
	PAPER_A5,
	PAPER_B4_ISO,
	PAPER_B5_ISO,
	PAPER_LETTER,
	PAPER_LEGAL,
	PAPER_TABLOID,
	PAPER_USER,
	PAPER_B6_ISO,
	PAPER_ENV_C4,
	PAPER_ENV_C5,
	PAPER_ENV_C6,
	PAPER_ENV_C65,
	PAPER_ENV_DL,
	PAPER_SLIDE_DIA,
	PAPER_SCREEN_4_3,
	PAPER_C,
	PAPER_D,
	PAPER_E,
	PAPER_EXECUTIVE,
	PAPER_FANFOLD_LEGAL_DE,
	PAPER_ENV_MONARCH,
	PAPER_ENV_PERSONAL,
	PAPER_ENV_9,
	PAPER_ENV_10,
	PAPER_ENV_11,
	PAPER_ENV_12,
	PAPER_KAI16,
	PAPER_KAI32,
	PAPER_KAI32BIG,
	PAPER_B4_JIS,
	PAPER_B5_JIS,
	PAPER_B6_JIS,
	PAPER_LEDGER,
	PAPER_STATEMENT,
	PAPER_QUARTO,
	PAPER_10x14,
	PAPER_ENV_14,
	PAPER_ENV_C3,
	PAPER_ENV_ITALY,
	PAPER_FANFOLD_US,
	PAPER_FANFOLD_DE,
	PAPER_POSTCARD_JP,
	PAPER_9x11,
	PAPER_10x11,
	PAPER_15x11,
	PAPER_ENV_INVITE,
	PAPER_A_PLUS,
	PAPER_B_PLUS,
	PAPER_LETTER_PLUS,
	PAPER_A4_PLUS,
	PAPER_DOUBLEPOSTCARD_JP,
	PAPER_A6,
	PAPER_12x11,
	PAPER_A7,
	PAPER_A8,
	PAPER_A9,
	PAPER_A10,
	PAPER_B0_ISO,
	PAPER_B1_ISO,
	PAPER_B2_ISO,
	PAPER_B3_ISO,
	PAPER_B7_ISO,
	PAPER_B8_ISO,
	PAPER_B9_ISO,
	PAPER_B10_ISO,
	PAPER_ENV_C2,
	PAPER_ENV_C7,
	PAPER_ENV_C8,
	PAPER_ARCHA,
	PAPER_ARCHB,
	PAPER_ARCHC,
	PAPER_ARCHD,
	PAPER_ARCHE,
	PAPER_SCREEN_16_9,
	PAPER_SCREEN_16_10,
	PAPER_16K_195x270,
	PAPER_16K_197x273,
	PAPER_WIDESCREEN, //PowerPoint Widescreen
	PAPER_ONSCREENSHOW_4_3, //PowerPoint On-screen Show (4:3)
	PAPER_ONSCREENSHOW_16_9, //PowerPoint On-screen Show (16:9)
	PAPER_ONSCREENSHOW_16_10, //PowerPoint On-screen Show (16:10)
}

const pageMarginOptions = {
	normal: {
		title: _('Normal'),
		icon: 'pagemargin',
		details: { Top: 0.79, Left: 0.79, Bottom: 0.79, Right: 0.79 },
	},
	wide: {
		title: _('Wide'),
		icon: 'pagemarginwide',
		details: { Top: 1, Left: 2, Bottom: 1, Right: 2 },
	},
	narrow: {
		title: _('Narrow'),
		icon: 'pagemarginnarrow',
		details: { Top: 0.5, Left: 0.5, Bottom: 0.5, Right: 0.5 },
	},
};

enum LO_BorderLineWidth {
	Hairline = 1, // 0.05pt
	VeryThin = 10, // 0.50pt
	Thin = 15, // 0.75pt
	Medium = 30, // 1.50pt
	Thick = 45, // 2.25pt
	ExtraThick = 90, // 4.50pt
}

enum UNO_BorderLineStyle {
	// Matches table::BorderLineStyle::DOUBLE in UNO IDL
	NONE = 32767,
	SOLID = 1,
	DOUBLE = 3,
}

function getLineStyleModificationCommand(
	LineStyle: UNO_BorderLineStyle,
	nOut: number, // outer line width, maps to SvxBorderLine nOut
	nIn: number, // inner line width, maps to SvxBorderLine nIn
	nDist: number, // distance between lines
): string {
	// The LineStyle property must be a BorderLine2 struct for
	// SvxLineItem::PutValue to parse it correctly (nMemId == 0).
	const params = {
		LineStyle: {
			type: 'com.sun.star.table.BorderLine2',
			value: {
				Color: { type: 'com.sun.star.util.Color', value: 0 },
				InnerLineWidth: { type: 'short', value: nIn },
				OuterLineWidth: { type: 'short', value: nOut },
				LineDistance: { type: 'short', value: nDist },
				LineStyle: { type: 'short', value: LineStyle },
				LineWidth: {
					type: 'unsigned long',
					value: nOut + nIn + nDist,
				},
			},
		},
	};

	const jsonParams = JSON.stringify(params);

	// The UNO command name itself, from `scslots.hxx`
	return `.uno:LineStyle ${jsonParams}`;
}
menuDefinitions.set('AutoSumMenu', [
	{ text: _('Sum'), uno: '.uno:AutoSum' },
	{ text: _('Average'), uno: '.uno:AutoSum?Function:string=average' },
	{ text: _('Min'), uno: '.uno:AutoSum?Function:string=min' },
	{ text: _('Max'), uno: '.uno:AutoSum?Function:string=max' },
	{ text: _('Count'), uno: '.uno:AutoSum?Function:string=count' },
] as Array<MenuDefinition>);

enum functionCategories {
	DATABASE = 1,
	DATEnTIME = 2,
	FINANCIAL = 3,
	INFORMATION = 4,
	LOGICAL = 5,
	MATHEMATICAL = 6,
	ARRAY = 7,
	STATISTICAL = 8,
	SPREADSHEET = 9,
	TEXT = 10,
}

const financialFunctions: Array<FunctionNameAlias> = [
	{ en: 'ACCRINT', de: 'AUFGELZINS', fr: 'INTERET.ACC', es: 'INT.ACUM' },
	{
		en: 'ACCRINTM',
		de: 'AUFGELZINSF',
		fr: 'INTERET.ACC.MAT',
		es: 'INT.ACUM.V',
	},
	{ en: 'AMORDEGRC', de: 'AMORDEGRK', fr: 'AMORDEGRC', es: 'AMORTIZ.PROGRE' },
	{ en: 'AMORLINC', de: 'AMORLINEARK', fr: 'AMORLINC', es: 'AMORTIZ.LIN' },
	{
		en: 'COUPDAYBS',
		de: 'ZINSTERMTAGVA',
		fr: 'NB.JOURS.COUPON.PREC',
		es: 'CUPON.DIAS.L1',
	},
	{
		en: 'COUPDAYS',
		de: 'ZINSTERMTAGE',
		fr: 'NB.JOURS.COUPONS',
		es: 'CUPON.DIAS',
	},
	{
		en: 'COUPDAYSNC',
		de: 'ZINSTERMTAGNZ',
		fr: 'NB.JOURS.COUPON.SUIV',
		es: 'CUPON.DIAS.L2',
	},
	{ en: 'COUPNUM', de: 'ZINSTERMZAHL', fr: 'NB.COUPONS', es: 'CUPON.NUM' },
	{
		en: 'COUPPCD',
		de: 'ZINSTERMVZ',
		fr: 'DATE.COUPON.PREC',
		es: 'CUPON.FECHA.L1',
	},
	{ en: 'CUMIPMT', de: 'KUMZINSZ', fr: 'CUMUL.INTER', es: 'PAGO.INT.ENTRE' },
	{ en: 'CUMIPMT_ADD', de: 'KUMZINSZ_ADD' },
	{
		en: 'CUMPRINC',
		de: 'KUMKAPITAL',
		fr: 'CUMUL.PRINCPER',
		es: 'PAGO.PRINC.ENTRE',
	},
	{ en: 'CUMPRINC_ADD', de: 'KUMKAPITAL_ADD' },
	{ en: 'DB', de: 'GDA2', fr: 'DB', es: 'DB' },
	{ en: 'DDB', de: 'GDA', fr: 'DDB', es: 'DDB' },
	{ en: 'DISC', de: 'DISAGIO', fr: 'TAUX.ESCOMPTE', es: 'TASA.DESC' },
	{ en: 'DOLLARDE', de: 'NOTIERUNGDEZ', fr: 'PRIX.DEC', es: 'MONEDA.DEC' },
	{ en: 'DOLLARFR', de: 'NOTIERUNGBRU', fr: 'PRIX.FRAC', es: 'MONEDA.FRAC' },
	{ en: 'DURATION', de: 'LAUFZEIT', fr: 'DUREE', es: 'DURACION' },
	{ en: 'EFFECT', de: 'EFFEKTIV', fr: 'TAUX.EFFECTIF', es: 'INT.EFECTIVO' },
	{ en: 'EFFECT_ADD', de: 'EFFEKTIV_ADD' },
	{ en: 'FV', de: 'Zielwert', fr: 'VC', es: 'vf' },
	{ en: 'FVSCHEDULE', de: 'ZW2', fr: 'VC.PAIEMENTS', es: 'VF.PLAN' },
	{ en: 'INTRATE', de: 'ZINSSATZ', fr: 'TAUX.INTERET', es: 'TASA.INT' },
	{ en: 'IPMT', de: 'ZINSZ', fr: 'INTPER', es: 'PAGOINT' },
	{ en: 'IRR', de: 'IKV', fr: 'TAUXRI', es: 'TIR' },
	{ en: 'ISPMT', de: 'ISPMT', fr: 'ISPMT', es: 'INT.PAGO.DIR' },
	{
		en: 'MDURATION',
		de: 'MLAUFZEIT',
		fr: 'DUREE.MODIFIEE',
		es: 'DURACION.MODIF',
	},
	{ en: 'MIRR', de: 'QIKV', fr: 'TRIM', es: 'TIRM' },
	{ en: 'NOMINAL', de: 'NOMINAL', fr: 'TAUX.NOMINAL', es: 'TASA.NOMINAL' },
	{ en: 'NOMINAL_ADD', de: 'NOMINAL_ADD' },
	{ en: 'NPER', de: 'Zeitraum', fr: 'NPER', es: 'NPER' },
	{ en: 'NPV', de: 'NBW', fr: 'VAN', es: 'VNA' },
	{
		en: 'ODDFPRICE',
		de: 'UNREGERKURS',
		fr: 'PRIX.PCOUPON.IRREG',
		es: 'PRECIO.PER.IRREGULAR.1',
	},
	{
		en: 'ODDFYIELD',
		de: 'UNREGERREND',
		fr: 'REND.PCOUPON.IRREG',
		es: 'RENDTO.PER.IRREGULAR.1',
	},
	{
		en: 'ODDLPRICE',
		de: 'UNREGLEKURS',
		fr: 'PRIX.DCOUPON.IRREG',
		es: 'PRECIO.PER.IRREGULAR.2',
	},
	{
		en: 'ODDLYIELD',
		de: 'UNREGLEREND',
		fr: 'REND.DCOUPON.IRREG',
		es: 'RENDTO.PER.IRREGULAR.2',
	},
	{
		en: 'OPT_BARRIER',
		de: 'OPT_BARRIER',
		fr: 'OPT_BARRIER',
		es: 'OPT_BARRIER',
	},
	{
		en: 'OPT_PROB_HIT',
		de: 'OPT_PROB_HIT',
		fr: 'OPT_PROB_HIT',
		es: 'OPT_PROB_HIT',
	},
	{
		en: 'OPT_PROB_INMONEY',
		de: 'OPT_PROB_INMONEY',
		fr: 'OPT_PROB_INMONEY',
		es: 'OPT_PROB_INMONEY',
	},
	{ en: 'OPT_TOUCH', de: 'OPT_TOUCH', fr: 'OPT_TOUCH', es: 'OPT_TOUCH' },
	{ en: 'PDURATION', de: 'PLAUFZEIT', fr: 'DUREE.PERIODE', es: 'P.DURACION' },
	{ en: 'PMT', de: 'Zahlungen', fr: 'VPM', es: 'PAGO' },
	{ en: 'PPMT', de: 'KAPZ', fr: 'PRINCPER', es: 'PAGOPRIN' },
	{ en: 'PRICE', de: 'KURS', fr: 'PRIX.TITRE', es: 'PRECIO' },
	{
		en: 'PRICEDISC',
		de: 'KURSDISAGIO',
		fr: 'VALEUR.ENCAISSEMENT',
		es: 'PRECIO.DESCUENTO',
	},
	{
		en: 'PRICEMAT',
		de: 'KURSFÄLLIG',
		fr: 'PRIX.TITRE.ECHEANCE',
		es: 'PRECIO.VENCIMIENTO',
	},
	{ en: 'PV', de: 'BW', fr: 'VA', es: 'VA' },
	{ en: 'RATE', de: 'ZINS', fr: 'TAUX', es: 'TASA' },
	{
		en: 'RECEIVED',
		de: 'AUSZAHLUNG',
		fr: 'VALEUR.NOMINALE',
		es: 'CANTIDAD.RECIBIDA',
	},
	{ en: 'RRI', de: 'ZGZ', fr: 'RENTINVEST', es: 'INT.RENDIMIENTO' },
	{ en: 'SLN', de: 'LIA', fr: 'AMORLIN', es: 'SLN' },
	{ en: 'SYD', de: 'DIA', fr: 'SYD', es: 'SYD' },
	{
		en: 'TBILLEQ',
		de: 'TBILLÄQUIV',
		fr: 'TAUX.ESCOMPTE.R',
		es: 'LETRA.DE.TES.EQV.A.BONO',
	},
	{
		en: 'TBILLPRICE',
		de: 'TBILLKURS',
		fr: 'PRIX.BON.TRESOR',
		es: 'LETRA.DE.TES.PRECIO',
	},
	{
		en: 'TBILLYIELD',
		de: 'TBILLRENDITE',
		fr: 'RENDEMENT.BON.TRESOR',
		es: 'LETRA.DE.TES.RENDTO',
	},
	{ en: 'XIRR', de: 'XINTZINSFUSS', fr: 'TRI.PAIEMENTS', es: 'TIR.NO.PER' },
	{ en: 'XNPV', de: 'XKAPITALWERT', fr: 'VAN.PAIEMENTS', es: 'VNA.NO.PER' },
	{ en: 'YIELD', de: 'RENDITE', fr: 'RENDEMENT.TITRE', es: 'RENDTO' },
	{
		en: 'YIELDDISC',
		de: 'RENDITEDIS',
		fr: 'RENDEMENT.SIMPLE',
		es: 'RENDTO.DESC',
	},
	{
		en: 'YIELDMAT',
		de: 'RENDITEFÄLL',
		fr: 'RENDEMENT.TITRE.ECHEANCE',
		es: 'RENDTO.VENCTO',
	},
];

const logicalFunctions: Array<FunctionNameAlias> = [
	{ en: 'AND', de: 'UND', fr: 'ET', es: 'Y' },
	{ en: 'FALSE', de: 'FALSCH', fr: 'FAUX', es: 'FALSO' },
	{ en: 'IF', de: 'WENN', fr: 'SI', es: 'SI' },
	{ en: 'IFERROR', de: 'WENNFEHLER', fr: 'SIERREUR', es: 'SI.ERROR' },
	{ en: 'IFNA', de: 'WENNNV', fr: 'SINA', es: 'SI.ND' },
	{ en: 'IFS', de: 'WENNS', fr: 'SI.CONDITIONS', es: 'SI.CONJUNTO' },
	{ en: 'NOT', de: 'NICHT', fr: 'NON', es: 'NO' },
	{ en: 'OR', de: 'ODER', fr: 'OU', es: 'O' },
	{ en: 'SWITCH', de: 'SCHALTER', fr: 'SI.MULTIPLE', es: 'CAMBIAR' },
	{ en: 'TRUE', de: 'WAHR', fr: 'VRAI', es: 'VERDADERO' },
	{ en: 'XOR', de: 'XODER', fr: 'XOR', es: 'XO' },
];

const textFunctions: Array<FunctionNameAlias> = [
	{ en: 'ARABIC', de: 'ARABISCH', fr: 'ARABE', es: 'NUMERO.ARABE' },
	{ en: 'ASC', de: 'ASC', fr: 'ASC', es: 'ASC' },
	{ en: 'BAHTTEXT', de: 'BAHTTEXT', fr: 'BAHTTEXTE', es: 'TEXTOBAHT' },
	{ en: 'BASE', de: 'BASIS', fr: 'BASE', es: 'BASE' },
	{ en: 'CHAR', de: 'ZEICHEN', fr: 'CAR', es: 'CARACTER' },
	{ en: 'CLEAN', de: 'SÄUBERN', fr: 'EPURAGE', es: 'LIMPIAR' },
	{ en: 'CODE', de: 'CODE', fr: 'CODE', es: 'CODIGO' },
	{ en: 'CONCAT', de: 'TEXTKETTE', fr: 'CONCAT', es: 'CONCAT' },
	{ en: 'CONCATENATE', de: 'VERKETTEN', fr: 'CONCATENER', es: 'CONCATENAR' },
	{ en: 'DECIMAL', de: 'DEZIMAL', fr: 'DECIMAL', es: 'DECIMAL' },
	{ en: 'DOLLAR', de: 'EUR', fr: 'FRANC', es: 'MONEDA' },
	{ en: 'ENCODEURL', de: 'URLCODIEREN', fr: 'ENCODEURL', es: 'URLCODIF' },
	{ en: 'FILTERXML', de: 'XMLFILTERN', fr: 'FILTREXML', es: 'XMLFILTRO' },
	{ en: 'FIND', de: 'FINDEN', fr: 'TROUVE', es: 'ENCONTRAR' },
	{ en: 'FINDB', de: 'FINDENB', fr: 'TROUVEB', es: 'ENCONTRARB' },
	{ en: 'FIXED', de: 'FEST', fr: 'CTXT', es: 'FIJO' },
	{ en: 'JIS', de: 'JIS', fr: 'JIS', es: 'JIS' },
	{ en: 'LEFT', de: 'LINKS', fr: 'GAUCHE', es: 'IZQUIERDA' },
	{ en: 'LEFTB', de: 'LINKSB', fr: 'GAUCHEB', es: 'IZQUIERDAB' },
	{ en: 'LEN', de: 'LÄNGE', fr: 'NBCAR', es: 'LARGO' },
	{ en: 'LENB', de: 'LÄNGEB', fr: 'NBCARB', es: 'LARGOB' },
	{ en: 'LOWER', de: 'KLEIN', fr: 'MINUSCULE', es: 'MINUSC' },
	{ en: 'MID', de: 'TEIL', fr: 'STXT', es: 'EXTRAE' },
	{ en: 'MIDB', de: 'TEILB', fr: 'STXTB', es: 'EXTRAEB' },
	{
		en: 'NUMBERVALUE',
		de: 'ZAHLWERT',
		fr: 'VALEUR.NOMBRE',
		es: 'VALOR.NUMERO',
	},
	{ en: 'PROPER', de: 'GROSS2', fr: 'NOMPROPRE', es: 'NOMPROPIO' },
	{ en: 'REGEX', de: 'REGAUS', fr: 'REGEX', es: 'EXP.REG' },
	{ en: 'REPLACE', de: 'ERSETZEN', fr: 'REMPLACER', es: 'REEMPLAZAR' },
	{ en: 'REPLACEB', de: 'ERSETZENB', fr: 'REMPLACERB', es: 'REEMPLAZARB' },
	{ en: 'REPT', de: 'WIEDERHOLEN', fr: 'REPT', es: 'REPETIR' },
	{ en: 'RIGHT', de: 'RECHTS', fr: 'DROITE', es: 'DERECHA' },
	{ en: 'RIGHTB', de: 'RECHTSB', fr: 'DROITEB', es: 'DERECHAB' },
	{ en: 'ROMAN', de: 'RÖMISCH', fr: 'ROMAIN', es: 'ROMANO' },
	{ en: 'ROT13', de: 'ROT13', fr: 'ROT13', es: 'ROT13' },
	{ en: 'SEARCH', de: 'SUCHEN', fr: 'CHERCHE', es: 'HALLAR' },
	{ en: 'SEARCHB', de: 'SUCHENB', fr: 'CHERCHEB', es: 'HALLARB' },
	{ en: 'SUBSTITUTE', de: 'WECHSELN', fr: 'SUBSTITUE', es: 'SUSTITUIR' },
	{ en: 'T', de: 'T', fr: 'T', es: 'T' },
	{ en: 'TEXT', de: 'TEXT', fr: 'TEXTE', es: 'TEXTO' },
	{ en: 'TEXTJOIN', de: 'VERBINDEN', fr: 'JOINDRE.TEXTE', es: 'UNIRCADENAS' },
	{ en: 'TRIM', de: 'GLÄTTEN', fr: 'SUPPRESPACE', es: 'ESPACIOS' },
	{ en: 'UNICHAR', de: 'UNIZEICHEN', fr: 'UNICAR', es: 'UNICAR' },
	{ en: 'UNICODE', de: 'UNICODE', fr: 'UNICODE', es: 'UNICODE' },
	{ en: 'UPPER', de: 'GROSS', fr: 'MAJUSCULE', es: 'MAYUSC' },
	{ en: 'VALUE', de: 'WERT', fr: 'CNUM', es: 'VALOR' },
	{ en: 'WEBSERVICE', de: 'WEBDIENST', fr: 'SERVICEWEB', es: 'SERVICIOWEB' },
];

const dateAndTimeFunctions: Array<FunctionNameAlias> = [
	{ en: 'DATE', de: 'DATUM', fr: 'DATE', es: 'FECHA' },
	{ en: 'DATEDIF', de: 'DATUMDIF', fr: 'DATEDIF', es: 'SIFECHA' },
	{ en: 'DATEVALUE', de: 'DATUMWERT', fr: 'DATEVAL', es: 'FECHANUMERO' },
	{ en: 'DAY', de: 'TAG', fr: 'JOUR', es: 'DIA' },
	{ en: 'DAYS', de: 'TAGE', fr: 'JOURS', es: 'DIAS' },
	{ en: 'DAYS360', de: 'TAGE360', fr: 'JOURS360', es: 'DIAS360' },
	{
		en: 'DAYSINMONTH',
		de: 'TAGEIMMONAT',
		fr: 'JOURSDANSMOIS',
		es: 'DIASENMES',
	},
	{ en: 'DAYSINYEAR', de: 'TAGEIMJAHR', fr: 'JOURSDANSANNEE', es: 'DIASENAÑO' },
	{
		en: 'EASTERSUNDAY',
		de: 'OSTERSONNTAG',
		fr: 'DIMANCHEDEPAQUES',
		es: 'DOMINGOPASCUA',
	},
	{ en: 'EDATE', de: 'EDATUM', fr: 'MOIS.DECALER', es: 'FECHA.MES' },
	{ en: 'EOMONTH', de: 'MONATSENDE', fr: 'FIN.MOIS', es: 'FIN.MES' },
	{ en: 'HOUR', de: 'STUNDE', fr: 'HEURE', es: 'HORA' },
	{
		en: 'ISLEAPYEAR',
		de: 'ISTSCHALTJAHR',
		fr: 'ESTBISSEXTILE',
		es: 'ESAÑOBISIESTO',
	},
	{
		en: 'ISOWEEKNUM',
		de: 'ISOKALENDERWOCHE',
		fr: 'NO.SEMAINE.ISO',
		es: 'ISO.NUM.DE.SEMANA',
	},
	{ en: 'MINUTE', de: 'MINUTE', fr: 'MINUTE', es: 'MINUTO' },
	{ en: 'MONTH', de: 'MONAT', fr: 'MOIS', es: 'MES' },
	{ en: 'MONTHS', de: 'MONATE', fr: 'NB.MOIS', es: 'MESES' },
	{
		en: 'NETWORKDAYS',
		de: 'NETTOARBEITSTAGE',
		fr: 'NB.JOURS.OUVRES',
		es: 'DIAS.LAB',
	},
	{
		en: 'NETWORKDAYS.INTL',
		de: 'NETTOARBEITSTAGE.INTL',
		fr: 'NB.JOURS.OUVRES.INTL',
		es: 'DIAS.LAB.INTL',
	},
	{ en: 'NOW', de: 'JETZT', fr: 'MAINTENANT', es: 'AHORA' },
	{ en: 'SECOND', de: 'SEKUNDE', fr: 'SECONDE', es: 'SEGUNDO' },
	{ en: 'TIME', de: 'ZEIT', fr: 'TEMPS', es: 'NSHORA' },
	{ en: 'TIMEVALUE', de: 'ZEITWERT', fr: 'TEMPSVAL', es: 'HORANUMERO' },
	{ en: 'TODAY', de: 'HEUTE', fr: 'AUJOURDHUI', es: 'HOY' },
	{ en: 'WEEKDAY', de: 'WOCHENTAG', fr: 'JOURSEM', es: 'DIASEM' },
	{ en: 'WEEKNUM', de: 'KALENDERWOCHE', fr: 'NO.SEMAINE', es: 'NUM.DE.SEMANA' },
	{ en: 'WEEKNUM_EXCEL2003', de: 'KALENDERWOCHE_EXCEL2003' },
	{
		en: 'WEEKNUM_OOO',
		de: 'KALENDERWOCHE_OOO',
		fr: 'NO.SEMAINE_OOO',
		es: 'NUM.DE.SEMANA_OOO',
	},
	{ en: 'WEEKS', de: 'WOCHEN', fr: 'SEMAINES', es: 'SEMANAS' },
	{
		en: 'WEEKSINYEAR',
		de: 'WOCHENIMJAHR',
		fr: 'SEMAINESDANSANNEE',
		es: 'SEMANASENAÑO',
	},
	{ en: 'WORKDAY', de: 'ARBEITSTAG', fr: 'SERIE.JOUR.OUVRE', es: 'DIA.LAB' },
	{
		en: 'WORKDAY.INTL',
		de: 'ARBEITSTAG.INTL',
		fr: 'SERIE.JOUR.OUVRE.INTL',
		es: 'DIA.LAB.INTL',
	},
	{ en: 'YEAR', de: 'JAHR', fr: 'ANNEE', es: 'AÑO' },
	{ en: 'YEARFRAC', de: 'BRTEILJAHRE', fr: 'FRACTION.ANNEE', es: 'FRAC.AÑO' },
	{ en: 'YEARS', de: 'JAHRE', fr: 'ANNEES', es: 'AÑOS' },
];

const lookupFunctions: Array<FunctionNameAlias> = [
	{ en: 'HLOOKUP', de: 'WVERWEIS', fr: 'RECHERCHEH', es: 'BUSCARH' },
	{ en: 'LOOKUP', de: 'VERWEIS', fr: 'RECHERCHE', es: 'BUSCAR' },
	{ en: 'VLOOKUP', de: 'SVERWEIS', fr: 'RECHERCHEV', es: 'BUSCARV' },
	{ en: 'XLOOKUP', de: 'XVERWEIS', fr: 'RECHERCHEX', es: 'BUSCARX' },
];

const refFunctions: Array<FunctionNameAlias> = [
	{ en: 'ISREF', de: 'ISTBEZUG', fr: 'ESTREF', es: 'ESREF' },
];

const mathAndTrigFunctions: Array<FunctionNameAlias> = [
	{ en: 'ABS', de: 'BETRAG', fr: 'ABS', es: 'ABS' },
	{ en: 'ACOS', de: 'ARCCOS', fr: 'ACOS', es: 'ACOS' },
	{ en: 'ACOSH', de: 'ARCCOSHYP', fr: 'ACOSH', es: 'ACOSH' },
	{ en: 'ACOT', de: 'ARCCOT', fr: 'ACOT', es: 'ACOT' },
	{ en: 'ACOTH', de: 'ARCCOTHYP', fr: 'ACOTH', es: 'ACOTH' },
	{ en: 'AGGREGATE', de: 'AGGREGAT', fr: 'AGREGAT', es: 'AGREGAR' },
	{ en: 'ASIN', de: 'ARCSIN', fr: 'ASIN', es: 'ASEN' },
	{ en: 'ASINH', de: 'ARCSINHYP', fr: 'ASINH', es: 'ASENH' },
	{ en: 'ATAN', de: 'ARCTAN', fr: 'ATAN', es: 'ATAN' },
	{ en: 'ATAN2', de: 'ARCTAN2', fr: 'ATAN2', es: 'ATAN2' },
	{ en: 'ATANH', de: 'ARCTANHYP', fr: 'ATANH', es: 'ATANH' },
	{ en: 'BITAND', de: 'BITUND', fr: 'BITAND', es: 'BIT.Y' },
	{
		en: 'BITLSHIFT',
		de: 'BITLVERSCHIEB',
		fr: 'BITLSHIFT',
		es: 'BIT.DESPLIZQDA',
	},
	{ en: 'BITOR', de: 'BITODER', fr: 'BITOR', es: 'BIT.O' },
	{
		en: 'BITRSHIFT',
		de: 'BITRVERSCHIEB',
		fr: 'BITRSHIFT',
		es: 'BIT.DESPLDCHA',
	},
	{ en: 'BITXOR', de: 'BITXODER', fr: 'BITXOR', es: 'BIT.XO' },
	{ en: 'CEILING', de: 'OBERGRENZE', fr: 'PLAFOND', es: 'MULTIPLO.SUPERIOR' },
	{
		en: 'CEILING.MATH',
		de: 'OBERGRENZE.MATHEMATIK',
		fr: 'PLAFOND.MATH',
		es: 'MULTIPLO.SUPERIOR.MAT',
	},
	{
		en: 'CEILING.PRECISE',
		de: 'OBERGRENZE.GENAU',
		fr: 'PLAFOND.PRECIS',
		es: 'MULTIPLO.SUPERIOR.EXACTO',
	},
	{
		en: 'CEILING.XCL',
		de: 'OBERGRENZE.EXCEL',
		fr: 'PLAFOND.XCL',
		es: 'MULTIPLO.SUPERIOR.XCL',
	},
	{ en: 'COLOR', de: 'FARBE', fr: 'COULEUR', es: 'COLOR' },
	{ en: 'COMBIN', de: 'KOMBINATIONEN', fr: 'COMBIN', es: 'COMBINAT' },
	{ en: 'COMBINA', de: 'KOMBINATIONEN2', fr: 'COMBINA', es: 'COMBINATA' },
	{
		en: 'CONVERT_OOO',
		de: 'UMRECHNEN_OOO',
		fr: 'CONVERTIR_OOO',
		es: 'CONVERTIR_OOO',
	},
	{ en: 'COS', de: 'COS', fr: 'COS', es: 'COS' },
	{ en: 'COSH', de: 'COSHYP', fr: 'COSH', es: 'COSH' },
	{ en: 'COT', de: 'COT', fr: 'COT', es: 'COT' },
	{ en: 'COTH', de: 'COTHYP', fr: 'COTH', es: 'COTH' },
	{ en: 'CSC', de: 'COSEC', fr: 'CSC', es: 'CSC' },
	{ en: 'CSCH', de: 'COSECHYP', fr: 'CSCH', es: 'CSCH' },
	{ en: 'DEGREES', de: 'GRAD', fr: 'DEGRES', es: 'GRADOS' },
	{
		en: 'EUROCONVERT',
		de: 'EUROUMRECHNEN',
		fr: 'EUROCONVERT',
		es: 'EUROCONVERT',
	},
	{ en: 'EVEN', de: 'GERADE', fr: 'PAIR', es: 'REDONDEA.PAR' },
	{ en: 'EXP', de: 'EXP', fr: 'EXP', es: 'EXP' },
	{ en: 'FACT', de: 'FAKULTÄT', fr: 'FACT', es: 'FACT' },
	{ en: 'FLOOR', de: 'UNTERGRENZE', fr: 'PLANCHER', es: 'MULTIPLO.INFERIOR' },
	{
		en: 'FLOOR.MATH',
		de: 'UNTERGRENZE.MATHEMATIK',
		fr: 'PLANCHER.MATH',
		es: 'MULTIPLO.INFERIOR.MAT',
	},
	{
		en: 'FLOOR.PRECISE',
		de: 'UNTERGRENZE.GENAU',
		fr: 'PLANCHER.PRECIS',
		es: 'MULTIPLO.INFERIOR.EXACTO',
	},
	{
		en: 'FLOOR.XCL',
		de: 'UNTERGRENZE.EXCEL',
		fr: 'PLANCHER.XCL',
		es: 'MULTIPLO.INFERIOR.XCL',
	},
	{ en: 'GCD', de: 'GGT', fr: 'PGCD', es: 'M.C.D' },
	{ en: 'GCD_EXCEL2003', de: 'GGT_EXCEL2003' },
	{ en: 'INT', de: 'GANZZAHL', fr: 'ENT', es: 'ENTERO' },
	{
		en: 'ISO.CEILING',
		de: 'ISO.OBERGRENZE',
		fr: 'ISO.PLAFOND',
		es: 'MULTIPLO.SUPERIOR.ISO',
	},
	{ en: 'LCM', de: 'KGV', fr: 'PPCM', es: 'M.C.M' },
	{ en: 'LCM_EXCEL2003', de: 'KGV_EXCEL2003' },
	{ en: 'LN', de: 'LN', fr: 'LN', es: 'LN' },
	{ en: 'LOG', de: 'LOG', fr: 'LOG', es: 'LOG' },
	{ en: 'LOG10', de: 'LOG10', fr: 'LOG10', es: 'LOG10' },
	{ en: 'MOD', de: 'REST', fr: 'MOD', es: 'RESIDUO' },
	{ en: 'MROUND', de: 'VRUNDEN', fr: 'ARRONDI.AU.MULTIPLE', es: 'REDOND.MULT' },
	{
		en: 'MULTINOMIAL',
		de: 'POLYNOMIAL',
		fr: 'MULTINOMIALE',
		es: 'MULTINOMIAL',
	},
	{ en: 'ODD', de: 'UNGERADE', fr: 'IMPAIR', es: 'REDONDEA.IMPAR' },
	{ en: 'PI', de: 'PI', fr: 'PI', es: 'PI' },
	{ en: 'POWER', de: 'POTENZ', fr: 'PUISSANCE', es: 'POTENCIA' },
	{ en: 'PRODUCT', de: 'PRODUKT', fr: 'PRODUIT', es: 'PRODUCTO' },
	{ en: 'QUOTIENT', de: 'QUOTIENT', fr: 'QUOTIENT', es: 'COCIENTE' },
	{ en: 'RADIANS', de: 'BOGENMASS', fr: 'RADIANS', es: 'RADIANES' },
	{ en: 'RAND', de: 'ZUFALLSZAHL', fr: 'ALEA', es: 'ALEATORIO' },
	{ en: 'RAND.NV', de: 'ZUFALLSZAHL.NF', fr: 'ALEA.NV', es: 'ALEATORIO.NV' },
	{
		en: 'RANDARRAY',
		de: 'ZUFALLSMATRIX',
		fr: 'TABLEAU.ALEA',
		es: 'MATRIZALEAT',
	},
	{
		en: 'RANDBETWEEN',
		de: 'ZUFALLSBEREICH',
		fr: 'ALEA.ENTRE.BORNES',
		es: 'ALEATORIO.ENTRE',
	},
	{
		en: 'RANDBETWEEN.NV',
		de: 'ZUFALLSBEREICH.NF',
		fr: 'ALEA.ENTRE.BORNES.NV',
		es: 'ALEATORIO.ENTRE.NV',
	},
	{ en: 'ROUND', de: 'RUNDEN', fr: 'ARRONDI', es: 'REDONDEAR' },
	{ en: 'ROUNDDOWN', de: 'ABRUNDEN', fr: 'ARRONDI.INF', es: 'REDONDEAR.MENOS' },
	{
		en: 'ROUNDSIG',
		de: 'RUNDENSIG',
		fr: 'ARRONDI_PRECISION',
		es: 'REDONDEAR.PRECISION',
	},
	{ en: 'ROUNDUP', de: 'AUFRUNDEN', fr: 'ARRONDI.SUP', es: 'REDONDEAR.MAS' },
	{ en: 'SEC', de: 'SEC', fr: 'SEC', es: 'SEC' },
	{ en: 'SECH', de: 'SECHYP', fr: 'SECH', es: 'SECH' },
	{ en: 'SERIESSUM', de: 'POTENZREIHE', fr: 'SOMME.SERIES', es: 'SUMA.SERIES' },
	{ en: 'SIGN', de: 'VORZEICHEN', fr: 'SIGNE', es: 'SIGNO' },
	{ en: 'SIN', de: 'SIN', fr: 'SIN', es: 'SEN' },
	{ en: 'SINH', de: 'SINHYP', fr: 'SINH', es: 'SENH' },
	{ en: 'SQRT', de: 'WURZEL', fr: 'RACINE', es: 'RAIZ' },
	{ en: 'SQRTPI', de: 'WURZELPI', fr: 'RACINE.PI', es: 'RAIZ2PI' },
	{ en: 'SUBTOTAL', de: 'TEILERGEBNIS', fr: 'SOUS.TOTAL', es: 'SUBTOTALES' },
	{ en: 'SUM', de: 'SUMME', fr: 'SOMME', es: 'SUMA' },
	{ en: 'SUMIF', de: 'SUMMEWENN', fr: 'SOMME.SI', es: 'SUMAR.SI' },
	{
		en: 'SUMIFS',
		de: 'SUMMEWENNS',
		fr: 'SOMME.SI.ENS',
		es: 'SUMAR.SI.CONJUNTO',
	},
	{
		en: 'SUMSQ',
		de: 'QUADRATESUMME',
		fr: 'SOMME.CARRES',
		es: 'SUMA.CUADRADOS',
	},
	{ en: 'TAN', de: 'TAN', fr: 'TAN', es: 'TAN' },
	{ en: 'TANH', de: 'TANHYP', fr: 'TANH', es: 'TANH' },
	{ en: 'TRUNC', de: 'KÜRZEN', fr: 'TRONQUE', es: 'TRUNCAR' },
];

const statisticalFunctions: Array<FunctionNameAlias> = [
	{ en: 'AVEDEV', de: 'MITTELABW', fr: 'ECART.MOYEN', es: 'DESVPROM' },
	{ en: 'AVERAGE', de: 'MITTELWERT', fr: 'MOYENNE', es: 'PROMEDIO' },
	{ en: 'AVERAGEA', de: 'MITTELWERTA', fr: 'MOYENNEA', es: 'PROMEDIOA' },
	{
		en: 'AVERAGEIF',
		de: 'MITTELWERTWENN',
		fr: 'MOYENNE.SI',
		es: 'PROMEDIO.SI',
	},
	{
		en: 'AVERAGEIFS',
		de: 'MITTELWERTWENNS',
		fr: 'MOYENNE.SI.ENS',
		es: 'PROMEDIO.SI.CONJUNTO',
	},
	{ en: 'B', de: 'B', fr: 'B', es: 'B' },
	{ en: 'BETA.DIST', de: 'BETA.VERT', fr: 'LOI.BETA', es: 'DISTR.BETA.N' },
	{ en: 'BETA.INV', de: 'BETA.INV', fr: 'BETA.INVERSE', es: 'INV.BETA.N' },
	{ en: 'BETADIST', de: 'BETAVERT', fr: 'LOIBETA', es: 'DISTR.BETA' },
	{ en: 'BETAINV', de: 'BETAINV', fr: 'BETAINVERSE', es: 'DISTR.BETA.INV' },
	{
		en: 'BINOM.DIST',
		de: 'BINOM.VERT',
		fr: 'LOI.BINOMIALE',
		es: 'DISTR.BINOM.N',
	},
	{
		en: 'BINOM.INV',
		de: 'BINOM.INV',
		fr: 'LOI.BINOMIALE.INVERSE.N',
		es: 'INV.BINOM',
	},
	{ en: 'BINOMDIST', de: 'BINOMVERT', fr: 'LOIBINOMIALE', es: 'DISTR.BINOM' },
	{ en: 'CHIDIST', de: 'CHIVERT', fr: 'LOIKHIDEUX', es: 'DISTR.CHI' },
	{ en: 'CHIINV', de: 'CHIINV', fr: 'KHIDEUX.INVERSE', es: 'PRUEBA.CHI.INV' },
	{
		en: 'CHISQ.DIST',
		de: 'CHIQU.VERT',
		fr: 'LOI.KHIDEUX.N',
		es: 'DISTR.CHICUAD',
	},
	{
		en: 'CHISQ.DIST.RT',
		de: 'CHIQU.VERT.RE',
		fr: 'LOI.KHIDEUX.DROITE',
		es: 'DISTR.CHICUAD.CD',
	},
	{
		en: 'CHISQ.INV',
		de: 'CHIQU.INV',
		fr: 'LOIKHIDEUX.INVERSE',
		es: 'INV.CHICUAD',
	},
	{
		en: 'CHISQ.INV.RT',
		de: 'CHIQU.INV.RE',
		fr: 'LOI.KHIDEUX.INVERSE.DROITE',
		es: 'INV.CHICUAD.CD',
	},
	{
		en: 'CHISQ.TEST',
		de: 'CHIQU.TEST',
		fr: 'TEST.LOI.KHIDEUX',
		es: 'PRUEBA.CHICUAD',
	},
	{ en: 'CHISQDIST', de: 'CHIQUVERT', fr: 'LOI.KHIDEUX', es: 'CHISQDIST' },
	{ en: 'CHISQINV', de: 'CHIQUINV', fr: 'LOIKHIDEUXINVERSE', es: 'CHISQINV' },
	{ en: 'CHITEST', de: 'CHITEST', fr: 'TEST.KHIDEUX', es: 'PRUEBA.CHI' },
	{
		en: 'CONFIDENCE',
		de: 'KONFIDENZ',
		fr: 'INTERVALLE.CONFIANCE',
		es: 'CONFIANZA',
	},
	{
		en: 'CONFIDENCE.NORM',
		de: 'KONFIDENZ.NORM',
		fr: 'INTERVALLE.CONFIANCE.NORMAL',
		es: 'INTERVALO.CONFIANZA.NORM',
	},
	{
		en: 'CONFIDENCE.T',
		de: 'KONFIDENZ.T',
		fr: 'INTERVALLE.CONFIANCE.T',
		es: 'INTERVALO.CONFIANZA.T',
	},
	{
		en: 'CORREL',
		de: 'KORREL',
		fr: 'COEFFICIENT.CORRELATION',
		es: 'COEF.DE.CORREL',
	},
	{ en: 'COUNT', de: 'ANZAHL', fr: 'NB', es: 'CONTAR' },
	{ en: 'COUNTA', de: 'ANZAHL2', fr: 'NBVAL', es: 'CONTARA' },
	{
		en: 'COUNTBLANK',
		de: 'ANZAHLLEEREZELLEN',
		fr: 'NB.VIDE',
		es: 'CONTAR.BLANCO',
	},
	{ en: 'COUNTIF', de: 'ZÄHLENWENN', fr: 'NB.SI', es: 'CONTAR.SI' },
	{
		en: 'COUNTIFS',
		de: 'ZÄHLENWENNS',
		fr: 'NB.SI.ENS',
		es: 'CONTAR.SI.CONJUNTO',
	},
	{ en: 'COVAR', de: 'KOVARIANZ', fr: 'COVARIANCE', es: 'COVAR' },
	{
		en: 'COVARIANCE.P',
		de: 'KOVARIANZ.P',
		fr: 'COVARIANCE.PEARSON',
		es: 'COVARIANZA.P',
	},
	{
		en: 'COVARIANCE.S',
		de: 'KOVARIANZ.S',
		fr: 'COVARIANCE.S',
		es: 'COVARIANZA.M',
	},
	{
		en: 'CRITBINOM',
		de: 'KRITBINOM',
		fr: 'CRITERE.LOI.BINOMIALE',
		es: 'BINOM.CRIT',
	},
	{ en: 'DEVSQ', de: 'SUMQUADABW', fr: 'SOMME.CARRES.ECARTS', es: 'DESVIA2' },
	{
		en: 'ERF.PRECISE',
		de: 'GAUSSF.GENAU',
		fr: 'ERF.PRECIS',
		es: 'FUN.ERROR.EXACTO',
	},
	{
		en: 'ERFC.PRECISE',
		de: 'GAUSSFKOMPL.GENAU',
		fr: 'ERFC.PRECIS',
		es: 'FUN.ERROR.COMPL.EXACTO',
	},
	{
		en: 'EXPON.DIST',
		de: 'EXPON.VERT',
		fr: 'LOI.EXPONENTIELLE',
		es: 'DISTR.EXP.N',
	},
	{ en: 'EXPONDIST', de: 'EXPONVERT', fr: 'LOIEXPONENTIELLE', es: 'DISTR.EXP' },
	{ en: 'F.DIST', de: 'F.VERT', fr: 'LOI.F', es: 'DIST.F' },
	{ en: 'F.DIST.RT', de: 'F.VERT.RE', fr: 'LOI.F.DROITE', es: 'DISTR.F.CD' },
	{ en: 'F.INV', de: 'F.INV', fr: 'INVERSE.LOI.F', es: 'F.INV' },
	{
		en: 'F.INV.RT',
		de: 'F.INV.RE',
		fr: 'INVERSE.LOI.F.DROITE',
		es: 'INV.F.CD',
	},
	{ en: 'F.TEST', de: 'F.TEST', fr: 'TEST.F', es: 'PRUEBA.F.N' },
	{ en: 'F.DIST', de: 'F.VERT', fr: 'LOI.F', es: 'DIST.F' },
	{ en: 'F.INV', de: 'F.INV', fr: 'INVERSE.LOI.F', es: 'F.INV' },
	{ en: 'FISHER', de: 'FISHER', fr: 'FISHER', es: 'FISHER' },
	{
		en: 'FISHERINV',
		de: 'FISHERINV',
		fr: 'FISHER.INVERSE',
		es: 'PRUEBA.FISHER.INV',
	},
	{ en: 'FORECAST', de: 'PROGNOSE', fr: 'PREVISION', es: 'PRONOSTICO' },
	{
		en: 'FORECAST.ETS.ADD',
		de: 'PROGNOSE.EXP.ADD',
		fr: 'PREVISION.ETS.ADD',
		es: 'PRONOSTICO.ETS.SUM',
	},
	{
		en: 'FORECAST.ETS.MULT',
		de: 'PROGNOSE.EXP.MULT',
		fr: 'PREVISION.ETS.MULT',
		es: 'PRONOSTICO.ETS.MULT',
	},
	{
		en: 'FORECAST.ETS.PI.ADD',
		de: 'PROGNOSE.EXP.VOR.ADD',
		fr: 'PREVISION.ETS.PI.ADD',
		es: 'PRONOSTICO.ETS.PI.SUM',
	},
	{
		en: 'FORECAST.ETS.PI.MULT',
		de: 'PROGNOSE.EXP.VOR.MULT',
		fr: 'PREVISION.ETS.PI.MULT',
		es: 'PRONOSTICO.ETS.PI.MULT',
	},
	{
		en: 'FORECAST.ETS.SEASONALITY',
		de: 'PROGNOSE.EXP.SAISONAL',
		fr: 'PREVISION.ETS.CARACTERESAISONNIER',
		es: 'PRONOSTICO.ETS.ESTACIONALIDAD',
	},
	{
		en: 'FORECAST.ETS.STAT.ADD',
		de: 'PROGNOSE.EXP.STAT.ADD',
		fr: 'PREVISION.ETS.STAT.ADD',
		es: 'PRONOSTICO.ETS.ESTADISTICA.SUM',
	},
	{
		en: 'FORECAST.ETS.STAT.MULT',
		de: 'PROGNOSE.EXP.STAT.MULT',
		fr: 'PREVISION.ETS.STAT.MULT',
		es: 'PRONOSTICO.ETS.ESTADISTICA.MULT',
	},
	{
		en: 'FORECAST.LINEAR',
		de: 'PROGNOSE.LINEAR',
		fr: 'PREVISION.LINEAIRE',
		es: 'PRONOSTICO.LINEAL',
	},
	{ en: 'FTEST', de: 'FTEST', fr: 'TESTF', es: 'PRUEBA.F' },
	{ en: 'GAMMA', de: 'GAMMA', fr: 'GAMMA', es: 'GAMMA' },
	{ en: 'GAMMA.DIST', de: 'GAMMA.VERT', fr: 'LOI.GAMMA', es: 'DISTR.GAMMA.N' },
	{
		en: 'GAMMA.INV',
		de: 'GAMMA.INV',
		fr: 'LOI.GAMMA.INVERSE',
		es: 'INV.GAMMA',
	},
	{ en: 'GAMMADIST', de: 'GAMMAVERT', fr: 'LOIGAMMA', es: 'DISTR.GAMMA' },
	{
		en: 'GAMMAINV',
		de: 'GAMMAINV',
		fr: 'LOIGAMMAINVERSE',
		es: 'DISTR.GAMMA.INV',
	},
	{ en: 'GAMMALN', de: 'GAMMALN', fr: 'LNGAMMA', es: 'GAMMA.LN' },
	{
		en: 'GAMMALN.PRECISE',
		de: 'GAMMALN.GENAU',
		fr: 'LNGAMMA.PRECIS',
		es: 'GAMMA.LN.EXACTO',
	},
	{ en: 'GAUSS', de: 'GAUSS', fr: 'GAUSS', es: 'GAUSS' },
	{
		en: 'GEOMEAN',
		de: 'GEOMITTEL',
		fr: 'MOYENNE.GEOMETRIQUE',
		es: 'MEDIA.GEOM',
	},
	{
		en: 'HARMEAN',
		de: 'HARMITTEL',
		fr: 'MOYENNE.HARMONIQUE',
		es: 'MEDIA.ARMO',
	},
	{
		en: 'HYPGEOM.DIST',
		de: 'HYPGEOM.VERT',
		fr: 'LOI.HYPERGEOMETRIQUE',
		es: 'DISTR.HIPERGEOM.N',
	},
	{
		en: 'HYPGEOMDIST',
		de: 'HYPGEOMVERT',
		fr: 'LOIHYPERGEOMETRIQUE',
		es: 'DISTR.HIPERGEOM',
	},
	{
		en: 'INTERCEPT',
		de: 'ACHSENABSCHNITT',
		fr: 'ORDONNEE.ORIGINE',
		es: 'INTERSECCION.EJE',
	},
	{ en: 'KURT', de: 'KURT', fr: 'KURTOSIS', es: 'CURTOSIS' },
	{ en: 'LARGE', de: 'KGRÖSSTE', fr: 'GRANDE.VALEUR', es: 'K.ESIMO.MAYOR' },
	{ en: 'LOGINV', de: 'LOGINV', fr: 'LOI.LOGNORMALE.INVERSE', es: 'INV.LOG' },
	{
		en: 'LOGNORM.DIST',
		de: 'LOGNORM.VERT',
		fr: 'LOI.LOGNORMALE',
		es: 'DISTR.LOGNORM',
	},
	{
		en: 'LOGNORM.INV',
		de: 'LOGNORM.INV',
		fr: 'LOI.LOGNORMALE.INVERSE.N',
		es: 'INV.LOGNORM',
	},
	{
		en: 'LOGNORMDIST',
		de: 'LOGNORMVERT',
		fr: 'LOILOGNORMALE',
		es: 'DISTR.LOG.NORM',
	},
	{ en: 'MAX', de: 'MAX', fr: 'MAX', es: 'MAX' },
	{ en: 'MAXA', de: 'MAXA', fr: 'MAXA', es: 'MAXA' },
	{ en: 'MAXIFS', de: 'MAXWENNS', fr: 'MAX.SI', es: 'MAX.SI.CONJUNTO' },
	{ en: 'MEDIAN', de: 'MEDIAN', fr: 'MEDIANE', es: 'MEDIANA' },
	{ en: 'MIN', de: 'MIN', fr: 'MIN', es: 'MIN' },
	{ en: 'MINA', de: 'MINA', fr: 'MINA', es: 'MINA' },
	{ en: 'MINIFS', de: 'MINWENNS', fr: 'MIN.SI', es: 'MIN.SI.CONJUNTO' },
	{ en: 'MODE', de: 'MODALWERT', fr: 'MODE', es: 'MODO' },
	{ en: 'MODE.SNGL', de: 'MODUS.EINF', fr: 'MODE.SIMPLE', es: 'MODA.UNO' },
	{
		en: 'NEGBINOM.DIST',
		de: 'NEGBINOM.VERT',
		fr: 'LOI.BINOMIALE.NEG',
		es: 'NEGBINOM.DIST',
	},
	{
		en: 'NEGBINOMDIST',
		de: 'NEGBINOMVERT',
		fr: 'LOIBINOMIALENEG',
		es: 'NEGBINOMDIST',
	},
	{
		en: 'NORMSDIST',
		de: 'STANDNORMVERT',
		fr: 'LOINORMALESTANDARD',
		es: 'DISTR.NORM.ESTAND',
	},
	{ en: 'NORM.DIST', de: 'NORM.VERT', fr: 'LOI.NORMALE', es: 'DISTR.NORM.N' },
	{
		en: 'NORMSDIST',
		de: 'STANDNORMVERT',
		fr: 'LOINORMALESTANDARD',
		es: 'DISTR.NORM.ESTAND',
	},
	{
		en: 'NORMSINV',
		de: 'STANDNORMINV',
		fr: 'LOINORMALESTANDARDINVERSE',
		es: 'DISTR.NORM.ESTAND.INV',
	},
	{ en: 'NORM.INV', de: 'NORM.INV', fr: 'LOI.NORMALE.INVERSE', es: 'INV.NORM' },
	{
		en: 'NORMSINV',
		de: 'STANDNORMINV',
		fr: 'LOINORMALESTANDARDINVERSE',
		es: 'DISTR.NORM.ESTAND.INV',
	},
	{
		en: 'NORM.S.DIST',
		de: 'NORM.S.VERT',
		fr: 'LOI.NORMALE.STANDARD',
		es: 'DISTR.NORM.ESTAND.N',
	},
	{
		en: 'NORM.S.INV',
		de: 'NORM.S.INV',
		fr: 'LOI.NORMALE.STANDARD.INVERSE',
		es: 'INV.NORM.ESTAND',
	},
	{ en: 'NORMDIST', de: 'NORMVERT', fr: 'LOINORMALE', es: 'DISTR.NORM' },
	{
		en: 'NORMINV',
		de: 'NORMINV',
		fr: 'LOINORMALEINVERSE',
		es: 'DISTR.NORM.INV',
	},
	{
		en: 'NORMSDIST',
		de: 'STANDNORMVERT',
		fr: 'LOINORMALESTANDARD',
		es: 'DISTR.NORM.ESTAND',
	},
	{
		en: 'NORMSINV',
		de: 'STANDNORMINV',
		fr: 'LOINORMALESTANDARDINVERSE',
		es: 'DISTR.NORM.ESTAND.INV',
	},
	{ en: 'PEARSON', de: 'PEARSON', fr: 'PEARSON', es: 'PEARSON' },
	{ en: 'PERCENTILE', de: 'QUANTIL', fr: 'CENTILE', es: 'PERCENTIL' },
	{
		en: 'PERCENTILE.EXC',
		de: 'QUANTIL.EXKL',
		fr: 'CENTILE.EXCLURE',
		es: 'PERCENTIL.EXC',
	},
	{
		en: 'PERCENTILE.INC',
		de: 'QUANTIL.INKL',
		fr: 'CENTILE.INCLURE',
		es: 'PERCENTIL.INC',
	},
	{
		en: 'PERCENTRANK',
		de: 'QUANTILSRANG',
		fr: 'RANG.POURCENTAGE',
		es: 'RANGO.PERCENTIL',
	},
	{
		en: 'PERCENTRANK.EXC',
		de: 'QUANTILSRANG.EXKL',
		fr: 'RANG.POURCENTAGE.EXCLURE',
		es: 'RANGO.PERCENTIL.EXC',
	},
	{
		en: 'PERCENTRANK.INC',
		de: 'QUANTILSRANG.INKL',
		fr: 'RANG.POURCENTAGE.INCLURE',
		es: 'RANGO.PERCENTIL.INC',
	},
	{ en: 'PERMUT', de: 'VARIATIONEN', fr: 'PERMUTATION', es: 'PERMUTACIONES' },
	{
		en: 'PERMUTATIONA',
		de: 'VARIATIONEN2',
		fr: 'PERMUTATIONA',
		es: 'PERMUTACIONESA',
	},
	{ en: 'PHI', de: 'PHI', fr: 'PHI', es: 'FI' },
	{ en: 'PROB', de: 'WAHRSCHBEREICH', fr: 'PROBABILITE', es: 'PROBABILIDAD' },
	{ en: 'QUARTILE', de: 'QUARTILE', fr: 'QUARTILE', es: 'CUARTIL' },
	{
		en: 'QUARTILE.EXC',
		de: 'QUARTILE.EXKL',
		fr: 'QUARTILE.EXCLURE',
		es: 'CUARTIL.EXC',
	},
	{
		en: 'QUARTILE.INC',
		de: 'QUARTILE.INKL',
		fr: 'QUARTILE.INCLURE',
		es: 'CUARTIL.INC',
	},
	{ en: 'RANK', de: 'RANG', fr: 'RANG', es: 'JERARQUIA' },
	{
		en: 'RANK.AVG',
		de: 'RANG.MITTELW',
		fr: 'MOYENNE.RANG',
		es: 'JERARQUIA.MEDIA',
	},
	{
		en: 'RANK.EQ',
		de: 'RANG.GLEICH',
		fr: 'EQUATION.RANG',
		es: 'JERARQUIA.EQV',
	},
	{
		en: 'RSQ',
		de: 'BESTIMMTHEITSMASS',
		fr: 'COEFFICIENT.DETERMINATION',
		es: 'COEFICIENTE.R2',
	},
	{
		en: 'SKEW',
		de: 'SCHIEFE',
		fr: 'COEFFICIENT.ASYMETRIE',
		es: 'COEFICIENTE.ASIMETRIA',
	},
	{
		en: 'SKEWP',
		de: 'SCHIEFEP',
		fr: 'COEFFICIENTASYMETRIEP',
		es: 'COEFICIENTE.ASIMETRIA.P',
	},
	{ en: 'SLOPE', de: 'STEIGUNG', fr: 'PENTE', es: 'PENDIENTE' },
	{ en: 'SMALL', de: 'KKLEINSTE', fr: 'PETITE.VALEUR', es: 'K.ESIMO.MENOR' },
	{
		en: 'STANDARDIZE',
		de: 'STANDARDISIERUNG',
		fr: 'CENTREE.REDUITE',
		es: 'NORMALIZACION',
	},
	{ en: 'STDEV', de: 'STABW', fr: 'ECARTYPE', es: 'DESVEST' },
	{ en: 'STDEV.P', de: 'STABW.N', fr: 'ECARTYPE.P', es: 'DESVEST.P' },
	{ en: 'STDEV.S', de: 'STABW.S', fr: 'ECARTYPE.S', es: 'DESVEST.M' },
	{ en: 'STDEVA', de: 'STABWA', fr: 'ECARTYPEA', es: 'DESVESTA' },
	{ en: 'STDEVP', de: 'STABWN', fr: 'ECARTYPEP', es: 'DESVESTP' },
	{ en: 'STDEVPA', de: 'STABWNA', fr: 'ECARTYPEPA', es: 'DESVESTPA' },
	{
		en: 'STEYX',
		de: 'STFEHLERYX',
		fr: 'ERREUR.TYPE.XY',
		es: 'ERROR.TIPICO.XY',
	},
	{ en: 'T.DIST', de: 'T.VERT', fr: 'LOI.STUDENT.N', es: 'DISTR.T.N' },
	{
		en: 'T.DIST.2T',
		de: 'T.VERT.2S',
		fr: 'LOI.STUDENT.BILATERALE',
		es: 'DISTR.T.2C',
	},
	{
		en: 'T.DIST.RT',
		de: 'T.VERT.RE',
		fr: 'LOI.STUDENT.DROITE',
		es: 'DISTR.T.CD',
	},
	{ en: 'T.INV', de: 'T.INV', fr: 'LOI.STUDENT.INVERSE', es: 'INV.T' },
	{
		en: 'T.INV.2T',
		de: 'T.INV.2S',
		fr: 'LOI.STUDENT.INVERSE.BILATERALE',
		es: 'INV.T.2C',
	},
	{ en: 'T.TEST', de: 'T.TEST', fr: 'TEST.STUDENT', es: 'PRUEBA.T.N' },
	{ en: 'TDIST', de: 'TVERT', fr: 'LOI.STUDENT', es: 'DISTR.T' },
	{ en: 'TINV', de: 'TINV', fr: 'LOISTUDENTINVERSE', es: 'DISTR.T.INV' },
	{
		en: 'TRIMMEAN',
		de: 'GESTUTZTMITTEL',
		fr: 'MOYENNE.REDUITE',
		es: 'MEDIA.ACOTADA',
	},
	{ en: 'TTEST', de: 'TTEST', fr: 'TESTSTUDENT', es: 'PRUEBA.T' },
	{ en: 'VAR', de: 'VARIANZ', fr: 'VAR', es: 'VAR' },
	{ en: 'VAR.P', de: 'VAR.P', fr: 'VAR.P', es: 'VAR.P' },
	{ en: 'VAR.S', de: 'VAR.S', fr: 'VAR.S', es: 'VAR.S' },
	{ en: 'VARA', de: 'VARIANZA', fr: 'VARA', es: 'VARA' },
	{ en: 'VARP', de: 'VARIANZEN', fr: 'VARP', es: 'VARP' },
	{ en: 'VARPA', de: 'VARIANZENA', fr: 'VAR.PA', es: 'VARPA' },
	{ en: 'WEIBULL', de: 'WEIBULL', fr: 'LOIWEIBULL', es: 'DIST.WEIBULL' },
	{
		en: 'WEIBULL.DIST',
		de: 'WEIBULL.VERT',
		fr: 'LOI.WEIBULL',
		es: 'DISTR.WEIBULL',
	},
	{ en: 'Z.TEST', de: 'G.TEST', fr: 'TEST.Z', es: 'PRUEBA.Z.N' },
	{ en: 'ZTEST', de: 'GTEST', fr: 'TESTZ', es: 'PRUEBA.Z' },
];

const databaseFunctions: Array<FunctionNameAlias> = [
	{ en: 'DAVERAGE', de: 'DBMITTELWERT', fr: 'BDMOYENNE', es: 'BDPROMEDIO' },
	{ en: 'DCOUNT', de: 'DBANZAHL', fr: 'BDNB', es: 'BDCONTAR' },
	{ en: 'DCOUNTA', de: 'DBANZAHL2', fr: 'BDNBVAL', es: 'BDCONTARA' },
	{ en: 'DGET', de: 'DBAUSZUG', fr: 'BDLIRE', es: 'BDEXTRAER' },
	{ en: 'DMAX', de: 'DBMAX', fr: 'BDMAX', es: 'BDMAX' },
	{ en: 'DMIN', de: 'DBMIN', fr: 'BDMIN', es: 'BDMIN' },
	{ en: 'DPRODUCT', de: 'DBPRODUKT', fr: 'BDPRODUIT', es: 'BDPRODUCTO' },
	{ en: 'DSTDEV', de: 'DBSTDABW', fr: 'BDECARTYPE', es: 'BDDESVEST' },
	{ en: 'DSUM', de: 'DBSUMME', fr: 'BDSOMME', es: 'BDSUMA' },
	{ en: 'DVAR', de: 'DBVARIANZ', fr: 'BDVAR', es: 'BDVAR' },
	{ en: 'DVARP', de: 'DBVARIANZEN', fr: 'BDVARP', es: 'BDVARP' },
];

const informationFunctions: Array<FunctionNameAlias> = [
	{ en: 'CELL', de: 'ZELLE', fr: 'CELLULE', es: 'CELDA' },
	{ en: 'CURRENT', de: 'AKTUELL', fr: 'ACTUELLE', es: 'ACTUAL' },
	{ en: 'FORMULA', de: 'FORMEL', fr: 'FORMULE', es: 'FORMULA' },
	{ en: 'INFO', de: 'INFO', fr: 'INFO', es: 'INFO' },
	{ en: 'ISBLANK', de: 'ISTLEER', fr: 'ESTVIDE', es: 'ESBLANCO' },
	{ en: 'ISERR', de: 'ISTFEHL', fr: 'ESTERR', es: 'ESERR' },
	{ en: 'ISERROR', de: 'ISTFEHLER', fr: 'ESTERREUR', es: 'ESERROR' },
	{ en: 'ISEVEN', de: 'ISTGERADE', fr: 'EST.PAIR', es: 'ESPAR' },
	{ en: 'ISEVEN_ADD', de: 'ISTGERADE_ADD' },
	{ en: 'ISFORMULA', de: 'ISTFORMEL', fr: 'ESTFORMULE', es: 'ESFORMULA' },
	{ en: 'ISLOGICAL', de: 'ISTLOGISCH', fr: 'ESTLOGIQUE', es: 'ESLOGICO' },
	{ en: 'ISNA', de: 'ISTNV', fr: 'ESTNA', es: 'ESNOD' },
	{ en: 'ISNONTEXT', de: 'ISTKEINTEXT', fr: 'ESTNONTEXTE', es: 'ESNOTEXTO' },
	{ en: 'ISNUMBER', de: 'ISTZAHL', fr: 'ESTNUM', es: 'ESNUMERO' },
	{ en: 'ISODD', de: 'ISTUNGERADE', fr: 'EST.IMPAIR', es: 'ES.IMPAR' },
	{ en: 'ISODD_ADD', de: 'ISTUNGERADE_ADD' },
	{ en: 'ISTEXT', de: 'ISTTEXT', fr: 'ESTTEXTE', es: 'ESTEXTO' },
	{ en: 'N', de: 'N', fr: 'N', es: 'N' },
	{ en: 'NA', de: 'NV', fr: 'NA', es: 'NOD' },
	{ en: 'TYPE', de: 'TYP', fr: 'TYPE', es: 'TIPO' },
];

const arrayFunctions: Array<FunctionNameAlias> = [
	{ en: 'FOURIER', de: 'FOURIER', fr: 'FOURIER', es: 'FOURIER' },
	{ en: 'FREQUENCY', de: 'HÄUFIGKEIT', fr: 'FREQUENCE', es: 'FRECUENCIA' },
	{ en: 'GROWTH', de: 'VARIATION', fr: 'CROISSANCE', es: 'CRECIMIENTO' },
	{ en: 'LINEST', de: 'RGP', fr: 'DROITEREG', es: 'ESTIMACION.LINEAL' },
	{ en: 'LOGEST', de: 'RKP', fr: 'LOGREG', es: 'ESTIMACION.LOGARITMICA' },
	{ en: 'MDETERM', de: 'MDET', fr: 'DETERMAT', es: 'MDETERM' },
	{ en: 'MINVERSE', de: 'MINV', fr: 'INVERSEMAT', es: 'MINVERSA' },
	{ en: 'MMULT', de: 'MMULT', fr: 'PRODUITMAT', es: 'MMULT' },
	{ en: 'MUNIT', de: 'EINHEITSMATRIX', fr: 'MUNIT', es: 'MUNITARIA' },
	{ en: 'SEQUENCE', de: 'FOLGE', fr: 'SEQUENCE', es: 'SECUENCIA' },
	{
		en: 'SUMPRODUCT',
		de: 'SUMMENPRODUKT',
		fr: 'SOMMEPROD',
		es: 'SUMA.PRODUCTO',
	},
	{ en: 'SUMX2MY2', de: 'SUMMEX2MY2', fr: 'SOMME.X2MY2', es: 'SUMAX2MENOSY2' },
	{ en: 'SUMX2PY2', de: 'SUMMEX2PY2', fr: 'SOMME.X2PY2', es: 'SUMAX2MASY2' },
	{ en: 'SUMXMY2', de: 'SUMMEXMY2', fr: 'SOMME.XMY2', es: 'SUMAXMENOSY2' },
	{ en: 'TRANSPOSE', de: 'MTRANS', fr: 'TRANSPOSE', es: 'TRANSPONER' },
	{ en: 'TREND', de: 'TREND', fr: 'TENDANCE', es: 'TENDENCIA' },
];

const spreadsheetFunctions: Array<FunctionNameAlias> = [
	{ en: 'ADDRESS', de: 'ADRESSE', fr: 'ADRESSE', es: 'DIRECCION' },
	{ en: 'AREAS', de: 'BEREICHE', fr: 'ZONES', es: 'AREAS' },
	{ en: 'CHOOSE', de: 'WAHL', fr: 'CHOISIR', es: 'ELEGIR' },
	{ en: 'COLUMN', de: 'SPALTE', fr: 'COLONNE', es: 'COLUMNA' },
	{ en: 'COLUMNS', de: 'SPALTEN', fr: 'COLONNES', es: 'COLUMNAS' },
	{ en: 'DDE', de: 'DDE', fr: 'DDE', es: 'DDE' },
	{
		en: 'ERROR.TYPE',
		de: 'FEHLER.TYP',
		fr: 'TYPE.ERREUR',
		es: 'TIPO.DE.ERROR',
	},
	{
		en: 'ERRORTYPE',
		de: 'FEHLERTYP',
		fr: 'TYPEERREUR',
		es: 'TIPO.DE.ERROR.OOO',
	},
	{ en: 'FILTER', de: 'FILTERN', fr: 'FILTRE', es: 'FILTRAR' },
	{
		en: 'GETPIVOTDATA',
		de: 'PIVOTDATENZUORDNEN',
		fr: 'EXTRAIRE.DONNEES.PILOTE',
		es: 'IMPORTARDATOSDINAMICOS',
	},
	{
		en: 'HYPERLINK',
		de: 'HYPERLINK',
		fr: 'LIEN.HYPERTEXTE',
		es: 'HIPERVINCULO',
	},
	{ en: 'INDEX', de: 'INDEX', fr: 'INDEX', es: 'INDICE' },
	{ en: 'INDIRECT', de: 'INDIREKT', fr: 'INDIRECT', es: 'INDIRECTO' },
	{ en: 'LET', de: 'LET', fr: 'LET', es: 'LET' },
	{ en: 'MATCH', de: 'VERGLEICH', fr: 'EQUIV', es: 'COINCIDIR' },
	{ en: 'OFFSET', de: 'VERSCHIEBUNG', fr: 'DECALER', es: 'DESREF' },
	{ en: 'ROW', de: 'ZEILE', fr: 'LIGNE', es: 'FILA' },
	{ en: 'ROWS', de: 'ZEILEN', fr: 'LIGNES', es: 'FILAS' },
	{ en: 'SHEET', de: 'TABELLE', fr: 'FEUILLE', es: 'HOJA' },
	{ en: 'SHEETS', de: 'TABELLEN', fr: 'FEUILLES', es: 'HOJAS' },
	{ en: 'SORT', de: 'SORTIEREN', fr: 'TRI', es: 'ORDENAR' },
	{ en: 'SORTBY', de: 'SORTIERENNACH', fr: 'TRIERPAR', es: 'ORDENARPOR' },
	{ en: 'STYLE', de: 'VORLAGE', fr: 'STYLE', es: 'ESTILO' },
	{ en: 'UNIQUE', de: 'EINZIGARTIG', fr: 'UNIQUE', es: 'UNICOS' },
];

function getLocalizedFuncName(funcNameList: FunctionNameAlias): string {
	if (String.locale.toLowerCase().startsWith('de') && funcNameList.de)
		return funcNameList.de;
	else if (String.locale.toLowerCase().startsWith('fr') && funcNameList.fr)
		return funcNameList.fr;
	else if (String.locale.toLowerCase().startsWith('es') && funcNameList.es)
		return funcNameList.es;

	return funcNameList.en;
}

function getFunctionsMenuArray(
	funcs: Array<FunctionNameAlias>,
	category: number,
): Array<MenuDefinition> {
	var functionsMenu: Array<MenuDefinition> = [];
	for (var func of funcs) {
		const localizedFunc = getLocalizedFuncName(func);
		functionsMenu.push({
			text: localizedFunc,
			uno:
				'.uno:InsertFunction?FunctionName:string=' +
				localizedFunc +
				'&FunctionCategory:short=' +
				category,
		} as MenuDefinition);
	}
	return functionsMenu;
}

menuDefinitions.set(
	'FinancialFunctionsMenu',
	getFunctionsMenuArray(financialFunctions, functionCategories.FINANCIAL),
);
menuDefinitions.set(
	'LogicalFunctionsMenu',
	getFunctionsMenuArray(logicalFunctions, functionCategories.LOGICAL),
);
menuDefinitions.set(
	'TextFunctionsMenu',
	getFunctionsMenuArray(textFunctions, functionCategories.TEXT),
);
menuDefinitions.set(
	'DateAndTimeFunctionsMenu',
	getFunctionsMenuArray(dateAndTimeFunctions, functionCategories.DATEnTIME),
);
menuDefinitions.set(
	'LookupAndRefFunctionsMenu',
	getFunctionsMenuArray(lookupFunctions, functionCategories.SPREADSHEET).concat(
		getFunctionsMenuArray(refFunctions, functionCategories.INFORMATION),
	),
);
menuDefinitions.set(
	'MathAndTrigFunctionsMenu',
	getFunctionsMenuArray(
		mathAndTrigFunctions,
		functionCategories.MATHEMATICAL,
	).concat(
		getFunctionsMenuArray(statisticalFunctions, functionCategories.STATISTICAL),
	),
);
menuDefinitions.set(
	'MoreFunctionsMenu',
	getFunctionsMenuArray(databaseFunctions, functionCategories.DATABASE).concat(
		getFunctionsMenuArray(informationFunctions, functionCategories.INFORMATION),
		getFunctionsMenuArray(arrayFunctions, functionCategories.ARRAY),
		getFunctionsMenuArray(spreadsheetFunctions, functionCategories.SPREADSHEET),
	),
);

menuDefinitions.set('Menu Statistic', [
	{
		text: _UNO('.uno:SamplingDialog', 'spreadsheet'),
		uno: '.uno:SamplingDialog',
	},
	{
		text: _UNO('.uno:DescriptiveStatisticsDialog', 'spreadsheet'),
		uno: '.uno:DescriptiveStatisticsDialog',
	},
	{
		text: _UNO('.uno:AnalysisOfVarianceDialog', 'spreadsheet'),
		uno: '.uno:AnalysisOfVarianceDialog',
	},
	{
		text: _UNO('.uno:CorrelationDialog', 'spreadsheet'),
		uno: '.uno:CorrelationDialog',
	},
	{
		text: _UNO('.uno:CovarianceDialog', 'spreadsheet'),
		uno: '.uno:CovarianceDialog',
	},
	{
		text: _UNO('.uno:ExponentialSmoothingDialog', 'spreadsheet'),
		uno: '.uno:ExponentialSmoothingDialog',
	},
	{
		text: _UNO('.uno:MovingAverageDialog', 'spreadsheet'),
		uno: '.uno:MovingAverageDialog',
	},
	{
		text: _UNO('.uno:RegressionDialog', 'spreadsheet'),
		uno: '.uno:RegressionDialog',
	},
	{ text: _UNO('.uno:TTestDialog', 'spreadsheet'), uno: '.uno:TTestDialog' },
	{ text: _UNO('.uno:FTestDialog', 'spreadsheet'), uno: '.uno:FTestDialog' },
	{ text: _UNO('.uno:ZTestDialog', 'spreadsheet'), uno: '.uno:ZTestDialog' },
	{
		text: _UNO('.uno:ChiSquareTestDialog', 'spreadsheet'),
		uno: '.uno:ChiSquareTestDialog',
	},
	{
		text: _UNO('.uno:FourierAnalysisDialog', 'spreadsheet'),
		uno: '.uno:FourierAnalysisDialog',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('EditSparklineMenu', [
	{
		text: _UNO('.uno:EditSparklineGroup', 'spreadsheet'),
		uno: 'EditSparklineGroup',
	},
	{
		text: _('Edit Single Sparkline'),
		uno: 'EditSparkline',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('DeleteSparklineMenu', [
	{
		text: _UNO('.uno:DeleteSparklineGroup', 'spreadsheet'),
		uno: 'DeleteSparklineGroup',
	},
	{
		text: _('Delete Single Sparkline'),
		uno: 'DeleteSparkline',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('MenuPrintRanges', [
	{
		text: _UNO('.uno:DefinePrintArea', 'spreadsheet'),
		uno: '.uno:DefinePrintArea',
	},
	{ text: _UNO('.uno:AddPrintArea', 'spreadsheet'), uno: '.uno:AddPrintArea' },
	{
		text: _UNO('.uno:EditPrintArea', 'spreadsheet'),
		uno: '.uno:EditPrintArea',
	},
	{
		text: _UNO('.uno:DeletePrintArea', 'spreadsheet'),
		uno: '.uno:DeletePrintArea',
	},
] as Array<MenuDefinition>);
menuDefinitions.set('MenuMargins', [
	{
		id: 'MarginMenu',
		type: 'json',
		content: {
			id: 'Layout-MarginMenu',
			type: 'pagemarginentry',
			options: pageMarginOptions,
			initialSelectedId: 'normal',
		},
	},
	{ id: 'MarginMenuSeparator', type: 'separator' },
] as Array<MenuDefinition>);

menuDefinitions.set('MenuOrientation', [
	{
		id: 'portrait',
		img: 'portrait',
		text: _('Portrait'),
		uno: '.uno:Orientation?isLandscape:bool=false',
	},
	{
		id: 'landscape',
		img: 'landscape',
		text: _('Landscape'),
		uno: '.uno:Orientation?isLandscape:bool=true',
	},
] as Array<MenuDefinition>);

const pageSizes = [
	{ id: 'A6', text: _('A6'), paper: Paper.PAPER_A6 },
	{ id: 'A5', text: _('A5'), paper: Paper.PAPER_A5 },
	{ id: 'A4', text: _('A4'), paper: Paper.PAPER_A4 },
	{ id: 'A3', text: _('A3'), paper: Paper.PAPER_A3 },
	{ id: 'B6ISO', text: _('B6 (ISO)'), paper: Paper.PAPER_B6_ISO },
	{ id: 'B5ISO', text: _('B5 (ISO)'), paper: Paper.PAPER_B5_ISO },
	{ id: 'B4ISO', text: _('B4 (ISO)'), paper: Paper.PAPER_B4_ISO },
	{ id: 'Letter', text: _('Letter'), paper: Paper.PAPER_LETTER },
	{ id: 'Legal', text: _('Legal'), paper: Paper.PAPER_LEGAL },
	{ id: 'LongBond', text: _('Long Bond'), paper: Paper.PAPER_FANFOLD_LEGAL_DE },
	{ id: 'Tabloid', text: _('Tabloid'), paper: Paper.PAPER_TABLOID },
	{ id: 'B6JIS', text: _('B6 (JIS)'), paper: Paper.PAPER_B6_JIS },
	{ id: 'B5JIS', text: _('B5 (JIS)'), paper: Paper.PAPER_B5_JIS },
	{ id: 'B4JIS', text: _('B4 (JIS)'), paper: Paper.PAPER_B4_JIS },
	{ id: '16Kai', text: _('16 Kai'), paper: Paper.PAPER_KAI16 },
	{ id: '32Kai', text: _('32 Kai'), paper: Paper.PAPER_KAI32 },
	{ id: 'Big32Kai', text: _('Big 32 Kai'), paper: Paper.PAPER_KAI32BIG },
	{ id: 'User', text: _('User'), paper: Paper.PAPER_USER },
	{ id: 'DLEnvelope', text: _('DL Envelope'), paper: Paper.PAPER_ENV_DL },
	{ id: 'C6Envelope', text: _('C6 Envelope'), paper: Paper.PAPER_ENV_C6 },
	{ id: 'C6_5Envelope', text: _('C6/5 Envelope'), paper: Paper.PAPER_ENV_C65 },
	{ id: 'C5Envelope', text: _('C5 Envelope'), paper: Paper.PAPER_ENV_C5 },
	{ id: 'C4Envelope', text: _('C4 Envelope'), paper: Paper.PAPER_ENV_C4 },
	{
		id: 'No6_3_4Envelope',
		text: _('#6¾ Envelope'),
		paper: Paper.PAPER_ENV_PERSONAL,
	},
	{
		id: 'No7_3_4MonarchEnvelope',
		text: _('#7¾ (Monarch) Envelope'),
		paper: Paper.PAPER_ENV_MONARCH,
	},
	{ id: 'No9Envelope', text: _('#9 Envelope'), paper: Paper.PAPER_ENV_9 },
	{ id: 'No10Envelope', text: _('#10 Envelope'), paper: Paper.PAPER_ENV_10 },
	{ id: 'No11Envelope', text: _('#11 Envelope'), paper: Paper.PAPER_ENV_11 },
	{ id: 'No12Envelope', text: _('#12 Envelope'), paper: Paper.PAPER_ENV_12 },
	{
		id: 'JapanesePostcard',
		text: _('Japanese Postcard'),
		paper: Paper.PAPER_POSTCARD_JP,
	},
];

menuDefinitions.set('MenuPageSizesCalc', [
	{
		id: 'PageSizeMenu',
		type: 'json',
		content: {
			id: 'Layout-PageSizeMenu',
			type: 'pagesizeentry',
			options: pageSizes.map((item) => ({
				id: item.id,
				text: item.text,
				uno: '.uno:CalcPageSize?PaperFormat:long=' + item.paper,
			})),
		},
	},
	{ id: 'PageSizeMenuSeparator', type: 'separator' },
] as Array<MenuDefinition>);

menuDefinitions.set(
	'MenuPageSizesWriter',
	pageSizes.map((item) => ({
		id: item.id,
		text: item.text,
		uno: '.uno:AttributePageSize?PaperFormat:short=' + item.paper,
	})) as Array<MenuDefinition>,
);

menuDefinitions.set('Print', [
	{ text: _('Active sheet'), action: 'print-active-sheet' },
	{ text: _('All Sheets'), action: 'print-all-sheets' },
] as Array<MenuDefinition>);

menuDefinitions.set('SheetListMenu', [
	{ type: 'separator' },
	{ type: 'separator' },
	// dynamically updated in Control.SheetsBar
] as Array<MenuDefinition>);

menuDefinitions.set('MenuRowHeight', [
	{ text: _UNO('.uno:RowHeight', 'spreadsheet'), uno: '.uno:RowHeight' },
	{
		text: _UNO('.uno:SetOptimalRowHeight', 'spreadsheet'),
		uno: '.uno:SetOptimalRowHeight',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('MenuColumnWidth', [
	{ text: _UNO('.uno:ColumnWidth', 'spreadsheet'), uno: '.uno:ColumnWidth' },
	{
		text: _UNO('.uno:SetOptimalColumnWidth', 'spreadsheet'),
		uno: '.uno:SetOptimalColumnWidth',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('FormattingMarkMenu', [
	{
		text: _UNO('.uno:InsertNonBreakingSpace', 'text'),
		uno: 'InsertNonBreakingSpace',
	},
	{ text: _UNO('.uno:InsertHardHyphen', 'text'), uno: 'InsertHardHyphen' },
	{ text: _UNO('.uno:InsertSoftHyphen', 'text'), uno: 'InsertSoftHyphen' },
	{ text: _UNO('.uno:InsertZWSP', 'text'), uno: 'InsertZWSP' },
	{ text: _UNO('.uno:InsertWJ', 'text'), uno: 'InsertWJ' },
	{ text: _UNO('.uno:InsertLRM', 'text'), uno: 'InsertLRM' },
	{ text: _UNO('.uno:InsertRLM', 'text'), uno: 'InsertRLM' },
] as Array<MenuDefinition>);

menuDefinitions.set('FormatMenu', [
	{ text: _UNO('.uno:Bold', 'text'), uno: 'Bold' },
	{ text: _UNO('.uno:Italic', 'text'), uno: 'Italic' },
	{ text: _UNO('.uno:Underline', 'text'), uno: 'Underline' },
	{ text: _UNO('.uno:UnderlineDouble', 'text'), uno: 'UnderlineDouble' },
	{ text: _UNO('.uno:Strikeout', 'text'), uno: 'Strikeout' },
	{ text: _UNO('.uno:Overline', 'text'), uno: 'Overline' },
	{ type: 'separator' },
	{ text: _UNO('.uno:SuperScript', 'text'), uno: 'SuperScript' },
	{ text: _UNO('.uno:SubScript', 'text'), uno: 'SubScript' },
	{ type: 'separator' },
	{ text: _UNO('.uno:Shadowed', 'text'), uno: 'Shadowed' },
	{ text: _UNO('.uno:OutlineFont', 'text'), uno: 'OutlineFont' },
	{ type: 'separator' },
	{ text: _UNO('.uno:Grow', 'text'), uno: 'Grow' },
	{ text: _UNO('.uno:Shrink', 'text'), uno: 'Shrink' },
	{ type: 'separator' },
	{ text: _UNO('.uno:ChangeCaseToUpper', 'text'), uno: 'ChangeCaseToUpper' },
	{ text: _UNO('.uno:ChangeCaseToLower', 'text'), uno: 'ChangeCaseToLower' },
	{
		text: _UNO('.uno:ChangeCaseRotateCase', 'text'),
		uno: 'ChangeCaseRotateCase',
	},
	{ type: 'separator' },
	{
		text: _UNO('.uno:ChangeCaseToSentenceCase', 'text'),
		uno: 'ChangeCaseToSentenceCase',
	},
	{
		text: _UNO('.uno:ChangeCaseToTitleCase', 'text'),
		uno: 'ChangeCaseToTitleCase',
	},
	{
		text: _UNO('.uno:ChangeCaseToToggleCase', 'text'),
		uno: 'ChangeCaseToToggleCase',
	},
	{ type: 'separator' },
	{ text: _UNO('.uno:SmallCaps', 'text'), uno: 'SmallCaps' },
] as Array<MenuDefinition>);

menuDefinitions.set('FormatBulletsMenu', [
	{ text: _UNO('.uno:DefaultBullet', 'text'), uno: 'DefaultBullet' },
	{ type: 'separator' },
	{ text: _UNO('.uno:DecrementLevel', 'text'), uno: 'DecrementLevel' },
	{ text: _UNO('.uno:IncrementLevel', 'text'), uno: 'IncrementLevel' },
	{ text: _UNO('.uno:DecrementSubLevels', 'text'), uno: 'DecrementSubLevels' },
	{ text: _UNO('.uno:IncrementSubLevels', 'text'), uno: 'IncrementSubLevels' },
	{ type: 'separator' },
	{ text: _UNO('.uno:MoveDown', 'text'), uno: 'MoveDown' },
	{ text: _UNO('.uno:MoveUp', 'text'), uno: 'MoveUp' },
	{ text: _UNO('.uno:MoveDownSubItems', 'text'), uno: 'MoveDownSubItems' },
	{ text: _UNO('.uno:MoveUpSubItems', 'text'), uno: 'MoveUpSubItems' },
	{ type: 'separator' },
	{
		text: _UNO('.uno:InsertNeutralParagraph', 'text'),
		uno: 'InsertNeutralParagraph',
	},
	{ text: _UNO('.uno:NumberingStart', 'text'), uno: 'NumberingStart' },
	{ text: _UNO('.uno:RemoveBullets', 'text'), uno: 'RemoveBullets' },
	{ type: 'separator' },
	{ text: _UNO('.uno:JumpDownThisLevel', 'text'), uno: 'JumpDownThisLevel' },
	{ text: _UNO('.uno:JumpUpThisLevel', 'text'), uno: 'JumpUpThisLevel' },
	{ text: _UNO('.uno:ContinueNumbering', 'text'), uno: 'ContinueNumbering' },
] as Array<MenuDefinition>);

menuDefinitions.set('LineSpacingMenu', [
	{
		id: 'spacepara1',
		img: 'spacepara1',
		text: _UNO('.uno:SpacePara1'),
		uno: 'SpacePara1',
	},
	{
		id: 'spacepara115',
		img: 'spacepara115',
		text: _UNO('.uno:SpacePara115'),
		uno: 'SpacePara115',
	},
	{
		id: 'spacepara15',
		img: 'spacepara15',
		text: _UNO('.uno:SpacePara15'),
		uno: 'SpacePara15',
	},
	{
		id: 'spacepara2',
		img: 'spacepara2',
		text: _UNO('.uno:SpacePara2'),
		uno: 'SpacePara2',
	},
	{ type: 'separator' },
	{
		id: 'paraspaceincrease',
		img: 'paraspaceincrease',
		text: _UNO('.uno:ParaspaceIncrease'),
		uno: 'ParaspaceIncrease',
	},
	{
		id: 'paraspacedecrease',
		img: 'paraspacedecrease',
		text: _UNO('.uno:ParaspaceDecrease'),
		uno: 'ParaspaceDecrease',
	},
	{ type: 'separator' },
	{
		id: 'paragraphdialog',
		img: 'paragraphdialog',
		text: _UNO('.uno:ParagraphDialog'),
		uno: 'ParagraphDialog',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('LanguageMenu', [
	{
		action: 'morelanguages-selection',
		text: _UNO('.uno:SetLanguageSelectionMenu', 'text'),
	},
	{
		action: 'morelanguages-paragraph',
		text: _UNO('.uno:SetLanguageParagraphMenu', 'text'),
	},
	{
		action: 'morelanguages-all',
		text: _UNO('.uno:SetLanguageAllTextMenu', 'text'),
	},
] as Array<MenuDefinition>);

menuDefinitions.set('InsertImageMenu', [
	{ action: 'localgraphic', text: _('Insert Local Image') },
	// local entry may be removed
	// remote entries added in Map.WOPI
] as Array<MenuDefinition>);

menuDefinitions.set('InsertMultimediaMenu', [
	{ action: 'insertmultimedia', text: _('Insert Local Multimedia') },
	// local entry may be removed
	// remote entries added in Map.WOPI
] as Array<MenuDefinition>);

menuDefinitions.set('CompareDocumentsMenu', [
	{ action: 'localcomparedocuments', text: _('Compare Local Document') },
	// local entry may be removed
	// remote entries added in Map.WOPI
] as Array<MenuDefinition>);

menuDefinitions.set('CharSpacingMenu', [
	{ id: 'space1', text: _('Very Tight'), uno: 'Spacing?Spacing:short=-60' },
	{ id: 'space1', text: _('Tight'), uno: 'Spacing?Spacing:short=-30' },
	{ id: 'space15', text: _('Normal'), uno: 'Spacing?Spacing:short=0' },
	{ id: 'space2', text: _('Loose'), uno: 'Spacing?Spacing:short=60' },
	{ id: 'space2', text: _('Very Loose'), uno: 'Spacing?Spacing:short=120' },
] as Array<MenuDefinition>);

menuDefinitions.set('PasteMenu', [
	{
		text: _UNO('.uno:Paste', 'text'),
		action: '.uno:Paste',
		hint: JSDialog.ShortcutsUtil.getShortcut(
			_UNO('.uno:Paste', 'text'),
			'.uno:Paste',
		),
	},
	{
		text: _UNO('.uno:PasteSpecial', 'text'),
		action: '.uno:PasteSpecial',
		hint: JSDialog.ShortcutsUtil.getShortcut(
			_UNO('.uno:PasteSpecial', 'text'),
			'.uno:PasteSpecial',
		),
	},
] as Array<MenuDefinition>);

menuDefinitions.set('ViewChangesMenu', [
	{
		id: 'viewchanges-inline',
		img: 'showtrackedchanges',
		text: _('Inline'),
		action: 'viewchanges-inline',
		checked: false,
	},
	{
		id: 'viewchanges-sidebyside',
		img: 'comparechanges',
		text: _('Side by Side'),
		action: 'viewchanges-sidebyside',
		checked: false,
	},
	{
		id: 'viewchanges-hidden',
		text: _('Hidden'),
		action: 'viewchanges-hidden',
		checked: false,
	},
] as Array<MenuDefinition>);

menuDefinitions.set('RecordTrackedChangesMenu', [
	{
		id: 'review-track-changes-off',
		text: _('Off'),
		uno: '.uno:TrackChanges?TrackChanges:bool=false',
	},
	{
		id: 'review-track-changes-all-views',
		text: _('All users'),
		uno: '.uno:TrackChangesInAllViews',
	},
	{
		id: 'review-track-changes-this-view',
		text: _('This user'),
		uno: '.uno:TrackChangesInThisView',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('AcceptTrackedChangesMenu', [
	{
		id: 'review-accept-tracked-change',
		text: _UNO('.uno:AcceptTrackedChange', 'text'),
		uno: '.uno:AcceptTrackedChange',
	},
	{
		id: 'review-accept-tracked-change-to-next',
		text: _UNO('.uno:AcceptTrackedChangeToNext', 'text'),
		uno: '.uno:AcceptTrackedChangeToNext',
	},
	{
		id: 'acceptalltrackedchanges',
		text: _UNO('.uno:AcceptAllTrackedChanges', 'text'),
		uno: '.uno:AcceptAllTrackedChanges',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('RejectTrackedChangesMenu', [
	{
		id: 'review-reject-tracked-change',
		text: _UNO('.uno:RejectTrackedChange', 'text'),
		uno: '.uno:RejectTrackedChange',
	},
	{
		id: 'review-reject-tracked-change-to-next',
		text: _UNO('.uno:RejectTrackedChangeToNext', 'text'),
		uno: '.uno:RejectTrackedChangeToNext',
	},
	{
		id: 'rejectalltrackedchanges',
		text: _UNO('.uno:RejectAllTrackedChanges', 'text'),
		uno: '.uno:RejectAllTrackedChanges',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('ConditionalFormatMenu', [
	{
		text: _('Highlight cells with...'),
		items: [
			{
				text: _('Values greater than...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=2',
			},
			{
				text: _('Values less than...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=1',
			},
			{
				text: _('Values equal to...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=0',
			},
			{
				text: _('Values between...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=6',
			},
			{
				text: _('Values duplicate...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=8',
			},
			{
				text: _('Containing text...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=23',
			},
			{ type: 'separator' },
			{ text: _('More highlights...'), uno: '.uno:ConditionalFormatDialog' },
		],
	},
	{
		text: _('Top/Bottom Rules...'),
		items: [
			{
				text: _('Top N elements...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=11',
			},
			{
				text: _('Top N percent...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=13',
			},
			{
				text: _('Bottom N elements...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=12',
			},
			{
				text: _('Bottom N percent...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=14',
			},
			{
				text: _('Above Average...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=15',
			},
			{
				text: _('Below Average...'),
				uno: '.uno:ConditionalFormatEasy?FormatRule:short=16',
			},
			{ type: 'separator' },
			{ text: _('More highlights...'), uno: '.uno:ConditionalFormatDialog' },
		],
	},
	{ type: 'separator' },
	{
		id: 'scaleset',
		text: _UNO('.uno:ColorScaleFormatDialog', 'spreadsheet'),
		items: [{ type: 'html', htmlId: 'scaleset' }],
	},
	{
		id: 'databarset',
		text: _UNO('.uno:DataBarFormatDialog', 'spreadsheet'),
		items: [{ type: 'html', htmlId: 'databarset' }],
	},
	{
		id: 'iconset',
		text: _UNO('.uno:IconSetFormatDialog', 'spreadsheet'),
		items: [{ type: 'html', htmlId: 'iconset' }],
	},
	{
		text: _UNO('.uno:CondDateFormatDialog', 'spreadsheet'),
		uno: '.uno:CondDateFormatDialog',
	},
	{ type: 'separator' },
	{
		text: _UNO('.uno:ConditionalFormatManagerDialog', 'spreadsheet'),
		uno: '.uno:ConditionalFormatManagerDialog',
	},
] as Array<MenuDefinition>);
menuDefinitions.set('BorderStyleMenu', [
	{
		id: 'noborders',
		img: 'fr01',
		text: _('No Borders'),
		uno: (window as any).getBorderStyleUNOCommand(0, 0, 0, 0, 0, 0, 0),
	},
	{
		id: 'leftborder',
		img: 'fr02',
		text: _('Left Border'),
		uno: (window as any).getBorderStyleUNOCommand(1, 0, 0, 0, 0, 0, 0),
	},
	{
		id: 'rightborder',
		img: 'fr03',
		text: _('Right Border'),
		uno: (window as any).getBorderStyleUNOCommand(0, 1, 0, 0, 0, 0, 0),
	},
	{
		id: 'leftandrightborders',
		img: 'fr04',
		text: _('Left And Right Borders'),
		uno: (window as any).getBorderStyleUNOCommand(1, 1, 0, 0, 0, 0, 0),
	},
	{
		id: 'topborder',
		img: 'fr05',
		text: _('Top Border'),
		uno: (window as any).getBorderStyleUNOCommand(0, 0, 0, 1, 0, 0, 0),
	},
	{
		id: 'bottomborder',
		img: 'fr06',
		text: _('Bottom Border'),
		uno: (window as any).getBorderStyleUNOCommand(0, 0, 1, 0, 0, 0, 0),
	},
	{
		id: 'topandbottomborder',
		img: 'fr07',
		text: _('Top And Bottom Border'),
		uno: (window as any).getBorderStyleUNOCommand(0, 0, 1, 1, 0, 0, 0),
	},
	{
		id: 'outerborder',
		img: 'fr08',
		text: _('Outer Border'),
		uno: (window as any).getBorderStyleUNOCommand(1, 1, 1, 1, 0, 0, 0),
	},
	{
		id: 'allhorizontallines',
		img: 'fr09',
		text: _('All Horizontal Lines'),
		uno: (window as any).getBorderStyleUNOCommand(0, 0, 1, 1, 1, 0, 0),
	},
	{
		id: 'outerborderandhorizontallines',
		img: 'fr010',
		text: _('Outer Border And Horizontal Lines'),
		uno: (window as any).getBorderStyleUNOCommand(1, 1, 1, 1, 1, 0, 0),
	},
	{
		id: 'outerborderandverticallines',
		img: 'fr011',
		text: _('Outer Border and Vertical Lines'),
		uno: (window as any).getBorderStyleUNOCommand(1, 1, 1, 1, 0, 1, 0),
	},
	{
		id: 'outerbordersandalllines',
		img: 'fr012',
		text: _('Outer Borders And All Inner lines'),
		uno: (window as any).getBorderStyleUNOCommand(1, 1, 1, 1, 1, 1, 0),
	},
	{ type: 'separator' },
	{
		text: _('Line color'),
		items: [
			{
				id: 'colorpickerwidget',
				type: 'colorpicker',
				command: '.uno:FrameLineColor',
			},
			{ type: 'separator' }, // required to show dropdown arrow
		],
	},
	{
		text: _('Line style'),
		items: [
			{
				text: _('Hairline (0.05 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.SOLID,
					LO_BorderLineWidth.Hairline,
					0,
					0,
				),
			},
			{
				text: _('Very thin (0.50 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.SOLID,
					LO_BorderLineWidth.VeryThin,
					0,
					0,
				),
			},
			{
				text: _('Thin (0.75 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.SOLID,
					LO_BorderLineWidth.Thin,
					0,
					0,
				),
			},
			{
				text: _('Medium (1.50 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.SOLID,
					LO_BorderLineWidth.Medium,
					0,
					0,
				),
			},
			{
				text: _('Thick (2.25 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.SOLID,
					LO_BorderLineWidth.Thick,
					0,
					0,
				),
			},
			{
				text: _('Extra thick (4.50 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.SOLID,
					LO_BorderLineWidth.ExtraThick,
					0,
					0,
				),
			},
			{
				text: _('Double Hairline (1.10 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.DOUBLE,
					LO_BorderLineWidth.Hairline,
					LO_BorderLineWidth.Hairline,
					LO_BorderLineWidth.Medium,
				),
			},
			{
				text: _('Double Hairline (2.35 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.DOUBLE,
					LO_BorderLineWidth.Hairline,
					LO_BorderLineWidth.Hairline,
					LO_BorderLineWidth.Thick,
				),
			},
			{
				text: _('Thin/Medium (3.00 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.DOUBLE,
					LO_BorderLineWidth.Thin,
					LO_BorderLineWidth.Medium,
					LO_BorderLineWidth.Thin,
				),
			},
			{
				text: _('Medium/Hairline (3.05 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.DOUBLE,
					LO_BorderLineWidth.Medium,
					LO_BorderLineWidth.Hairline,
					LO_BorderLineWidth.Medium,
				),
			},
			{
				text: _('Medium/Medium (4.50 pt)'),
				uno: getLineStyleModificationCommand(
					UNO_BorderLineStyle.DOUBLE,
					LO_BorderLineWidth.Medium,
					LO_BorderLineWidth.Medium,
					LO_BorderLineWidth.Medium,
				),
			},
			{ type: 'separator' }, // required to show dropdown arrow
		],
	},
	{ type: 'separator' },
	{
		id: 'more',
		text: _('More...'),
		uno: '.uno:FormatCellBorders',
	},
	{ type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set(
	'BorderStyleMenuWriter',
	(menuDefinitions.get('BorderStyleMenu') || []).filter(
		(item) => item.uno !== '.uno:FormatCellBorders',
	),
);

menuDefinitions.set('InsertShapesMenu', [
	{ type: 'html', htmlId: 'insertshapespopup' },
	{ type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set('InsertConnectorsMenu', [
	{ type: 'html', htmlId: 'insertconnectorspopup' },
	{ type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set('InsertTableMenu', [
	{ type: 'html', htmlId: 'inserttablepopup' },
	{ type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set('UsersListMenu', [
	{ type: 'html', htmlId: 'userslistpopup' },
	{ type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set('ColorPickerMenu', [
	{ id: 'colorpickerwidget', type: 'colorpicker' },
	{ type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set('LanguageStatusMenu', [
	{ type: 'separator' },
	{ type: 'separator' },
	// dynamically updated in Control.StatusBar
] as Array<MenuDefinition>);

menuDefinitions.set('SlideSizeMenu', [
	{
		text: _('Standard (4:3)'),
		img: 'standard-size',
		uno: '.uno:AttributePageSize?AttributePageSize.Width:long=28000&AttributePageSize.Height:long=21000',
	},
	{
		text: _('Widescreen (16:9)'),
		img: 'widescreen-size',
		uno: '.uno:AttributePageSize?AttributePageSize.Width:long=28000&AttributePageSize.Height:long=15750',
	},
] as Array<MenuDefinition>);

function generateLayoutPopupGrid(unoCommand: string): GridWidgetJSON {
	// please see enum AutoLayout in autolayout.hxx. this is the actual WhatLayout sequence
	// based on the visual position of the icons in the popup.
	const layoutMap = [
		{ layout: 20, text: _('Blank Slide') },
		{ layout: 0, text: _('Title Slide') },
		{ layout: 1, text: _('Title, Content') },
		{ layout: 3, text: _('Title and 2 Content') },
		{ layout: 19, text: _('Title Only') },
		{ layout: 32, text: _('Centered Text') },
		{ layout: 15, text: _('Title, 2 Content and Content') },
		{ layout: 12, text: _('Title, Content and 2 Content') },
		{ layout: 16, text: _('Title, 2 Content over Content') },
		{ layout: 14, text: _('Title, Content over Content') },
		{ layout: 18, text: _('Title, 4 Content') },
		{ layout: 34, text: _('Title, 6 Content') },
		{ layout: 27, text: _('Vertical Title, Text, Chart') },
		{ layout: 28, text: _('Vertical Title, Vertical Text') },
		{ layout: 29, text: _('Title, Vertical Text') },
		{ layout: 30, text: _('Title, Vertical Text, Clipart') },
	];

	const grid = {
		id: 'slidelayoutgrid',
		type: 'grid',
		cols: 4,
		rows: 4,
		children: new Array<WidgetJSON>(),
	};

	for (let i = 0; i < 16; i += 4) {
		for (let j = i; j < i + 4; j++) {
			grid.children.push({
				id: 'layout' + j,
				type: 'toolitem',
				command:
					'.uno:' + unoCommand + '?WhatLayout:long=' + layoutMap[j].layout,
				text: layoutMap[j].text,
				noLabel: true,
				left: j % 4,
				top: (i / 4) % 4,
				tabIndex: 0,
				index: Math.floor(i / 4) + ':' + (j % 4),
			} as any as WidgetJSON);
		}
	}

	return grid as any as GridWidgetJSON;
}

function generatePictureBrightnessMenu(
	unoCommand: string,
): Array<MenuDefinition> {
	const brightnessValues = [-40, -20, 0, 20, 40];

	const menuItems: Array<MenuDefinition> = [];

	for (let i = 0; i < brightnessValues.length; i++) {
		menuItems.push({
			id: 'brightness' + brightnessValues[i],
			uno: '.uno:' + unoCommand + '?Brightness:short=' + brightnessValues[i],
			text: brightnessValues[i] + '%',
			img: 'insertgraphic',
		} as MenuDefinition);
	}

	return menuItems;
}

function generatePictureContrastMenu(
	unoCommand: string,
): Array<MenuDefinition> {
	const contrastValues = [-40, -20, 0, 20, 40];

	const menuItems: Array<MenuDefinition> = [];

	for (let i = 0; i < contrastValues.length; i++) {
		menuItems.push({
			id: 'contrast' + contrastValues[i],
			uno: '.uno:' + unoCommand + '?Contrast:short=' + contrastValues[i],
			text: contrastValues[i] + '%',
			img: 'insertgraphic',
		} as MenuDefinition);
	}

	return menuItems;
}

function generatePictureTransparencyMenu(
	unoCommand: string,
): Array<MenuDefinition> {
	const transparencyValues = [0, 15, 30, 50, 65, 80, 95];

	const menuItems: Array<MenuDefinition> = [];

	for (let i = 0; i < transparencyValues.length; i++) {
		menuItems.push({
			id: 'transparency' + transparencyValues[i],
			uno:
				'.uno:' + unoCommand + '?Transparency:short=' + transparencyValues[i],
			text: transparencyValues[i] + '%',
			img: 'insertgraphic',
		} as MenuDefinition);
	}

	return menuItems;
}

menuDefinitions.set('NewSlideLayoutMenu', [
	{
		id: 'NewSlideLayoutMenu',
		type: 'json',
		content: {
			id: 'Layout-NewSlideLayoutMenu',
			// _UNO('.uno:InsertCanvasSlide')
			// Keep the above comment for unocommands.py
			type: 'newslidelayoutentry',
			gridContent: generateLayoutPopupGrid('InsertPage'),
		},
	},
	{ id: 'NewSlideLayoutMenuSeparator', type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set('ChangeSlideLayoutMenu', [
	{
		id: 'ChangeSlideLayoutMenu',
		type: 'json',
		content: generateLayoutPopupGrid('AssignLayout'),
	},
	{ id: 'ChangeSlideLayoutMenuSeparator', type: 'separator' }, // required to show dropdown arrow
] as Array<MenuDefinition>);

menuDefinitions.set(
	'PictureBrightness',
	generatePictureBrightnessMenu('GrafLuminance'),
);

menuDefinitions.set(
	'PictureContrast',
	generatePictureContrastMenu('GrafContrast'),
);

menuDefinitions.set(
	'PictureTransparency',
	generatePictureTransparencyMenu('GrafTransparence'),
);

menuDefinitions.set('PictureColorMode', [
	{
		text: _('Default'),
		img: 'insertgraphic',
		uno: '.uno:GrafMode?ColorMode:short=0',
	},
	{
		text: _('Grayscale'),
		img: 'insertgraphic',
		uno: '.uno:GrafMode?ColorMode:short=1',
	},
	{
		text: _('Black/White'),
		img: 'insertgraphic',
		uno: '.uno:GrafMode?ColorMode:short=2',
	},
	{
		text: _('Watermark'),
		img: 'insertgraphic',
		uno: '.uno:GrafMode?ColorMode:short=3',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('AnnotationMenu', [
	{
		text: _('Insert comment'),
		action: 'insertcomment',
	},
	{
		text: _('Comments list'),
		action: 'showcommentsnavigator',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('CalcAnnotationMenu', [
	{
		text: _('Insert Comment'),
		action: 'insertthreadedcomment',
	},
	{
		text: _('Insert Note'),
		action: 'insertcomment',
	},
	{
		text: _('Comments list'),
		action: 'showcommentsnavigator',
	},
] as Array<MenuDefinition>);

menuDefinitions.set('PictureEffectsMenu', [
	{
		text: _UNO('.uno:GraphicFilterInvert'),
		uno: '.uno:GraphicFilterInvert',
		img: 'graphicfilterinvert',
	},
	{
		text: _UNO('.uno:GraphicFilterSharpen'),
		uno: '.uno:GraphicFilterSharpen',
		img: 'graphicfiltersharpen',
	},
	{
		text: _UNO('.uno:GraphicFilterRemoveNoise'),
		uno: '.uno:GraphicFilterRemoveNoise',
		img: 'graphicfilterremovenoise',
	},
	{
		text: _UNO('.uno:GraphicFilterPopart'),
		uno: '.uno:GraphicFilterPopart',
		img: 'graphicfilterpopart',
	},
	{
		text: _UNO('.uno:GraphicFilterSobel'),
		uno: '.uno:GraphicFilterSobel',
		img: 'graphicfiltersobel',
	},
] as Array<MenuDefinition>);

JSDialog.MenuDefinitions = menuDefinitions;
